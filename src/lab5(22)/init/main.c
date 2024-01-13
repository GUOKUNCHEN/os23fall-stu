#include "printk.h"
#include "sbi.h"
#include "clock.h"
extern char _stext[];
extern char _srodata[];

extern void schedule();

int start_kernel()
{
	// printk("kernel is running!\n");
	printk("[S-MODE] Hello RISC-V\n");
	clock_set_next_event();
   	schedule();
	test(); // DO NOT DELETE !!!

	return 0;
}
