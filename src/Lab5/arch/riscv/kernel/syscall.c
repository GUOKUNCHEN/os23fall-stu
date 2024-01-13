#include "printk.h"
#include "proc.h"
#include "syscall.h"

extern struct task_struct *current;
extern struct pt_regs* regs;

/* 此处fd为标准输出（1），buf为用户需要打印的起始地址，count为字符串长度，返回打印的字符数。 */
uint64 sys_write(unsigned int fd, const char* buf, size_t count){
	uint64 res=0;
	if(fd==1U){
		for(int i=0;i<count;i++)
		{
			printk("%c",buf[i]);
			res++;
		}
	}
	return res;
}

/* 该调用从current中获取当前的pid放入a0中返回，无参数。 */
uint64 sys_getpid()
{
	return current->pid;
}

void syscall(struct pt_regs* regs)
{
	switch(regs->x[17])//从getpid.c中可知SYSCALL的ID放在了a7中，同时
	{
		case SYS_WRITE:
		{
			((char*)(regs->x[11]))[regs->x[12]] = '\0';
			regs->x[10]=sys_write(regs->x[10],(const char*)(regs->x[11]),regs->x[12]);//x10为a0,x11为a1，均是进入trap前的内容
			break;
		}
		case SYS_GETPID:
		{
			regs->x[10]=sys_getpid();
			break;
		}
		default:break;
	}
}
