// trap.c
#include "printk.h"
#include "clock.h"
#include "proc.h"

extern void syscall(struct pt_regs *regs);

void trap_handler(unsigned long scause, unsigned long sepc, struct pt_regs *regs)
{
	if (scause & (1LL << 63))
	{
		if (scause ^ (1LL << 63) == 5)
		{
			// printk("kernel is running!\n");
			// printk("[S] Supervisor Mode Timer Interrupt\n");
			clock_set_next_event();
			do_timer();
		}
	}
	else
	{//通过查询scause表得知当触发ECALL_FROM_U_MODE异常时，scause的值为8
		if (scause == 8)
		{
			syscall(regs);
			//regs->sepc+=4;//这是为了让程序从traps结束时执行异常PC位置的下一条指令，保证程序的正常执行
		}
	}
}
