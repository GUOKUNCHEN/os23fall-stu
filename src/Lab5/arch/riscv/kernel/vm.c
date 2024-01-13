// arch/riscv/kernel/vm.c
#include "defs.h"
#include "types.h"
extern char _stext[];
extern char _srodata[];
extern char _sdata[];
extern char _etext[];
extern char _erodata[];
extern char _edata[];


#define PPN_BIT_SIZE 44
#define PPN_SIZE (0x1 << (PPN_BIT_SIZE))
#define PERM_BIT_SIZE 10
#define PG_LEVEL_MAX 3
#define PG_OFF_BIT_SIZE 12
#define VPN_BIT_SIZE 9
#define VPN_SIZE (0x1 << (VPN_BIT_SIZE))

/* 这里的level最低为1，此时只移位PAGE_OFFSET个bit */
#define VPN(level, addr) \
	(((addr >> (PG_OFF_BIT_SIZE + (level - 1) * VPN_BIT_SIZE))) & (VPN_SIZE - 1))

/* early_pgtbl: 用于 setup_vm 进行 1GB 的 映射。 */
unsigned long early_pgtbl[512] __attribute__((__aligned__(0x1000)));
void setup_vm(void);
void setup_vm_final(void);
int create_mapping(uint64 *pgtbl, uint64 va, uint64 pa, uint64 sz, int perm);

void setup_vm(void)
{
	/*
	1. 由于是进行 1GB 的映射 这里不需要使用多级页表
	2. 将 va 的 64bit 作为如下划分： | high bit | 9 bit | 30 bit |
		high bit 可以忽略
		中间9 bit 作为 early_pgtbl 的 index
		低 30 bit 作为 页内偏移 这里注意到 30 = 9 + 9 + 12， 即我们只使用根页表， 根页表的每个 entry 都对应 1GB 的区域。
	3. Page Table Entry 的权限 V | R | W | X 位设置为 1
	*/

	memset((void *)early_pgtbl, 0x0, PGSIZE);
	// 等值映射
	uint64 pa = PHY_START, va = PHY_START; // PPN[2]是26位，所以&0x3ffffff
	early_pgtbl[VPN(3, va)] = (((pa >> 30) & 0x3ffff) << 28) | (PTE_V | PTE_R | PTE_W | PTE_X);
	// 高地址映射
	va += PA2VA_OFFSET;
	early_pgtbl[VPN(3, va)] = (((pa >> 30) & 0x3ffff) << 28) | (PTE_V | PTE_R | PTE_W | PTE_X);
}

// arch/riscv/kernel/vm.c

/* swapper_pg_dir: kernel pagetable 根目录， 在 setup_vm_final 进行映射。 */
unsigned long swapper_pg_dir[512] __attribute__((__aligned__(0x1000)));

void setup_vm_final(void)
{
	memset(swapper_pg_dir, 0x0, PGSIZE);
	uint64 pa = PHY_START + OPENSBI_SIZE, va = VM_START + OPENSBI_SIZE;
	// No OpenSBI mapping required
	// mapping kernel text X|-|R|V
	create_mapping(swapper_pg_dir, va, pa, ((uint64)_srodata - (uint64)_stext), PTE_X | PTE_R | PTE_V);
	va += (uint64)_srodata - (uint64)_stext;
	pa += (uint64)_srodata - (uint64)_stext;

	// mapping kernel rodata -|-|R|V
	create_mapping(swapper_pg_dir, va, pa, ((uint64)_sdata - (uint64)_srodata), PTE_R | PTE_V);
	va += (uint64)_sdata - (uint64)_srodata;
	pa += (uint64)_sdata - (uint64)_srodata;

	// mapping other memory -|W|R|V
	create_mapping(swapper_pg_dir, va, pa, (PHY_SIZE - ((uint64)_sdata - (uint64)_stext)), PTE_W | PTE_R | PTE_V);

	// set satp with swapper_pg_dir
	uint64 PHY_pg_dir = (uint64)swapper_pg_dir - PA2VA_OFFSET;
	asm volatile(
		"mv t0,%[PHY_pg_dir]\n"
		"srli t0,t0,12\n"
		"li t1,(0x8<<60)\n"
		"add t0,t0,t1\n"
		"csrw satp,t0\n" ::[PHY_pg_dir] "r"(PHY_pg_dir)
		: "memory");
	// YOUR CODE HERE

	// flush TLB
	asm volatile("sfence.vma zero,zero\n");
	asm volatile("fence.i\n");
	return;
}

/* 创建多级页表映射关系 */
int create_mapping(uint64 *root_pgtbl, uint64 va, uint64 pa, uint64 sz, int perm)
{
	/*
	pgtbl 为根页表的基地址
	va, pa 为需要映射的虚拟地址、物理地址
	sz 为映射的大小
	perm 为映射的读写权限

	创建多级页表的时候可以使用 kalloc() 来获取一页作为页表目录
	可以使用 V bit 来判断页表项是否存在
	*/
	uint64 index;
	uint64 addr = va;
	uint64 end = va + sz;
	uint64 *Tb = root_pgtbl;
	uint64 *temp;
	// last = (uint64 *)((pgtbl + sz) &);
	uint64 level;
	while (addr < end)
	{
		Tb = (uint64 *)root_pgtbl;
		level = PG_LEVEL_MAX;
		while (level > 1)
		{
			index = VPN(level, (uint64)addr);
			// printk("%llx\n", Tb[index]);
			if ((Tb[index] & PTE_V) == PTE_V) // 页项有效,表明空间存在且权限已设置好
			{
				Tb = (uint64 *)((Tb[index] >> 10) << 12); // 进行指针寻址，进入到下一级页表，寻址不应考虑权限位
			}
			else if ((Tb[index] & PTE_V) == 0) // 页项无效，需要申请空间建立新页，同时设置权限
			{
				temp = kalloc();
				temp = ((((uint64)temp - PA2VA_OFFSET) >> 12) << 10) | PTE_V;
				Tb[index] = temp;
				Tb = (uint64 *)(((Tb[index] >> 10) << 12) + PA2VA_OFFSET); // 随后修改参考页表，进行下一级检索
			}
			else
			{
				printk("Error in creating mapping!\n");
			}
			level--;
		}
		// 此时level为1级(VPN[0]),直接从对应的Page Table就可取得实际页地址
		// 所以此时用物理地址设置PPN后，设置权限即可
		index = VPN(level, (uint64)addr);
		Tb[index] = ((pa >> 12) << 10) | perm;
		// 到这里就完成一次映射
		// 接下来进行下一页映射的建立
		addr = addr + PGSIZE;
		pa = pa + PGSIZE;
	}

	return;
}
