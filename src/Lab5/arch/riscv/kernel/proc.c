/* Lab3 added */
// arch/riscv/kernel/proc.c
#define DSJF
#define __TEST__
#include "defs.h"
#include "mm.h"
#include "printk.h"
#include "proc.h"

extern char uapp_start[];															 // 用户空间开始，定义位置在vmlinux.lds.S中
extern char uapp_end[];																 // 用户空间结束，定义位置在vmlinux.lds.S中
extern uint64 swapper_pg_dir[512];													 // 内核页表，外部引用，具体位置在vm.c中
extern int create_mapping(uint64 *pgtbl, uint64 va, uint64 pa, uint64 sz, int perm); // 建立映射函数，外部引用，具体位置在vm.c中
extern uint64 rand();

extern void __dummy();
extern void __switch_to(struct task_struct *prev, struct task_struct *next);

struct task_struct *idle;			// idle process
struct task_struct *current;		// 指向当前运行线程的 `task_struct`
struct task_struct *task[NR_TASKS]; // 线程数组, 所有的线程都保存在此

void task_init()
{
	// 1. 调用 kalloc() 为 idle 分配一个物理页
	// 2. 设置 state 为 TASK_RUNNING;
	// 3. 由于 idle 不参与调度 可以将其 counter / priority 设置为 0
	// 4. 设置 idle 的 pid 为 0
	// 5. 将 current 和 task[0] 指向 idle

	// 分配物理页
#ifdef __TEST__
	__asm__ volatile(
		"task_first_init:\n");
#endif
	idle = (struct task_struct *)kalloc();

	// 设置 state 为 TASK_RUNNING;
	idle->state = TASK_RUNNING;

	// 由于 idle 不参与调度 可以将其 counter / priority 设置为 0
	idle->counter = 0;
	idle->priority = 0;

	// 设置 idle 的 pid 为 0
	idle->pid = 0;

	// 将 current 和 task[0] 指向 idle
	current = idle;
	task[0] = idle;
	// uint64 page_head = kalloc();
	// idle = (struct task_struct *)page_head;
	// uint64 sp_pos;
	/* 分界线，上面为去年的 */
	// // 将sp栈指针设置到物理页高地址部分
	// //  uint64 sp_pos = page_head + 0xfff;
	// //  __asm__ volatile(
	// //  	"mv sp,%[sp_pos]\n" ::[sp_pos] "r"(sp_pos)
	// //  	: "memory");
	// // 设置task_struct中的前半部分，（其实不用存...因为已经初始化过了）
	// __asm__ volatile(
	// 	"mv t0,%[page_head]\n" ::[page_head] "r"(page_head)
	// 	: "memory");
	// // 设置thread_struct内容,由于物理页已经被初始化过，寄存器全都重置了，只需要记录ra和sp即可
	// __asm__ volatile(
	// 	"sd ra,40(t0)\n"
	// 	"sd sp,48(t0)\n" ::
	// 		: "memory");

	// current = (struct task_struct *)page_head;
	// task[0] = current;

	/* YOUR CODE HERE */

	// 1. 参考 idle 的设置, 为 task[1] ~ task[NR_TASKS - 1] 进行初始化
	// 2. 其中每个线程的 state 为 TASK_RUNNING, counter 为 0, priority 使用 rand() 来设置, pid 为该线程在线程数组中的下标。
	// 3. 为 task[1] ~ task[NR_TASKS - 1] 设置 `thread_struct` 中的 `ra` 和 `sp`,
	// 4. 其中 `ra` 设置为 __dummy （见 4.3.2）的地址,  `sp` 设置为 该线程申请的物理页的高地址

	uint64 i;
	for (i = 1; i < NR_TASKS; i++)
	{
		task[i] = (struct task_struct *)kalloc();
		// 其中每个线程的 state 为 TASK_RUNNING, counter 为 0
		// priority 使用 rand() 来设置, pid 为该线程在线程数组中的下标。
		task[i]->state = TASK_RUNNING;
		task[i]->counter = 0;
		task[i]->priority = rand() % (PRIORITY_MAX - PRIORITY_MIN + 1) + PRIORITY_MIN;
		task[i]->pid = i;
		// 为 task[1] ~ task[NR_TASKS - 1] 设置 `thread_struct` 中的 `ra` 和 `sp`
		// 其中 `ra` 设置为 __dummy 的地址
		// `sp` 设置为 该线程申请的物理页的高地址
		task[i]->thread.ra = (uint64)__dummy;
		task[i]->thread.sp = (uint64)(task[i]) + PGSIZE;

		/* 以下为Lab5添加部分 */
		// 用kalloc申请一个用户页作为U-mode Stack,同时设定好kernel_sp
		task[i]->thread_info = (uint64)kalloc();
		task[i]->thread_info->kernel_sp = task[i]->thread.sp;
		task[i]->thread_info->user_sp = (uint64)kalloc();
		// 下面创建页表
		task[i]->pgd = (pagetable_t)kalloc();
		// 随后为了避免 U-Mode 和 S-Mode 切换的时候切换页表，先将内核页表swapper_pg_dir 复制到每个进程的页表中
		// uint64 table[512];
		for (int j = 0; j < 512; j++)
		{
			if (swapper_pg_dir[j])
				task[i]->pgd[j] = swapper_pg_dir[j];
			// table[j] = task[i]->pgd[j];
		}

		// 接着需要进行uapp的映射
		uint64 va = USER_START;
		uint64 pa = (uint64)(uapp_start)-PA2VA_OFFSET;
		create_mapping((uint64 *)task[i]->pgd, va, pa, (uint64)(uapp_end) - (uint64)(uapp_start), PTE_U | PTE_X | PTE_W | PTE_R | PTE_V);

		//	随后映射U-Stack,将 U-Mode Stack 映射至虚拟地址空间的USER_END-PGSIZE位置（即用户空间的最后一个页面），权限设置为 U | - | W | R | V
		va = USER_END - PGSIZE;
		pa = task[i]->thread_info->user_sp - PA2VA_OFFSET;
		create_mapping(task[i]->pgd, va, pa, PGSIZE, PTE_U | PTE_W | PTE_R | PTE_V);

		//	接着修改sepc，设置sstatus的SPP，SPIE，SUM，同时sscratch设置为U-Mode的sp,其值位USER_END
		task[i]->thread.sepc = USER_START;
		task[i]->thread.sstatus = csr_read(sstatus);
		// sstatus的8号位(SPP)设置为0以启动U-Mode的返回状态
		task[i]->thread.sstatus &= ~(1UL << 8);
		// sstatus的5号位(SPIE)设置为1以实现在sret后开启中断
		task[i]->thread.sstatus |= (1UL << 5);
		// sstatus的18号位(SUM(Supervisor User Memory access))设置为1以实现在S-Mode下能够访问User的页面
		task[i]->thread.sstatus |= (1UL << 18);
		// sscratch设置为U-Mode的sp
		task[i]->thread.sscratch = USER_END;
		// 随后将此时已经完成映射的页表pgd以satp寄存器的数据格式存入线程数据结构中，方便后续切换页表的操作
		uint64 satp = csr_read(satp);
		satp = (satp >> 44) << 44;									   // 取PPN[43:0]
		satp |= ((uint64)(task[i]->pgd) - (uint64)PA2VA_OFFSET) >> 12; // va到pa的转换
		task[i]->pgd = (pagetable_t)satp;							   // 用pgd存储对应的satp，这样页表切换的时候只需要恢复和保存后加载即可

		/* Lab5添加部分结束 */
	}
	/* YOUR CODE HERE */

	printk("...proc_init done!\n");
	for (i = 1; i < NR_TASKS; i++)
	{
		if (i)
			task[i]->counter = rand() % 10; // 控制在10以内，不然太大了
#ifdef __TEST__
		printk("SET [PID = %d COUNTER = %d]\n", task[i]->pid, task[i]->counter);
#endif
	}
#ifdef __TEST__
	__asm__ volatile(
		"task_init_end:\n");
#endif
	// sp_pos=current->thread.ra;
	// __asm__ volatile(
	// 	"mv ra,%[ret_addr]\n"
	// 	"ret\n" ::[ret_addr] "r"(sp_pos)
	// 	: "memory");
}

void dummy()
{
	uint64 MOD = 1000000007;
	uint64 auto_inc_local_var = 0;
	int last_counter = -1;
	// do_timer();
	while (1)
	{
		if (last_counter == -1 || current->counter != last_counter)
		{
			last_counter = current->counter;
			auto_inc_local_var = (auto_inc_local_var + 1) % MOD;
			// printk("[PID = %d] is running. auto_inc_local_var = %d\n", current->pid, auto_inc_local_var);
			printk("[U-Mode][PID = %d][sp = 0x%lx] Print Counter:%d\n", current->pid, current->thread.sp, auto_inc_local_var);
		}
	}
	return;
}

void switch_to(struct task_struct *next)
{
	/* YOUR CODE HERE */
	if (current == next)
	{
		printk("The two Processes are the same.\n");
	}
	else
	{
#ifdef __TEST__
		printk("switch to [PID = %d COUNTER = %d]\n", next->pid, next->counter);
#endif
		// 注意这里current将会改变，在汇编中处理变量比较麻烦，所以就直接在这里处理了
		struct task_struct *prev = current;
		current = next;
		__switch_to(prev, next);
	}
}

void do_timer(void)
{
	// 1. 如果当前线程是 idle 线程 直接进行调度
	// 2. 如果当前线程不是 idle 对当前线程的运行剩余时间减1 若剩余时间仍然大于0 则直接返回 否则进行调度
	if (current == idle)
	{
		schedule();
	}
	else
	{
		current->counter--;
		if (current->counter > 0)
		{

			//	clock_set_next_event();
			return;
		}
		else
			schedule();
	}
	/* YOUR CODE HERE */
}

// Schedule Algorithm Implementation
#ifdef DSJF
void schedule(void)
{

	struct task_struct *next_task = NULL;
	int i;
	uint64 min_time = 0xffffffff;
	while (next_task == NULL)
	{
		for (i = 1; i < NR_TASKS; i++)
		{
			if (task[i]->state == TASK_RUNNING && task[i]->counter > 0)
				if (min_time > task[i]->counter)
				{
					min_time = task[i]->counter;
					next_task = task[i];
				}
		}

		// 如果所有线程的运行剩余时间都为0，则此时next_task保持为NULL状态，我们就需要重新随机化。

		if (next_task == NULL)
		{
			printk("Here reset all P's counter\n");
			for (i = 1; i < NR_TASKS; i++)
			{
				task[i]->counter = rand() % 10; // 控制在10以内，不然太大了
				printk("SET [PID = %d COUNTER = %d]\n", task[i]->pid, task[i]->counter);
			}
		}
	}

	switch_to(next_task);

	/* YOUR CODE HERE */
}
#endif
#ifdef DPRIORITY
void schedule(void)
{
	/* YOUR CODE HERE */
}
#endif