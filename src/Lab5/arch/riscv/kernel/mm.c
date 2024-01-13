#include "defs.h"
#include "string.h"
#include "mm.h"

#include "printk.h"

extern char _ekernel[];

struct
{
	struct run *freelist;
} kmem;

uint64 kalloc()
{
	struct run *r;

	//从freelist中取出一片地址，赋给r
	r = kmem.freelist;
	kmem.freelist = r->next;
	//对r的内存内容初始化为0
	memset((void *)r, 0x0, PGSIZE);
	return (uint64)r;
}

void kfree(uint64 addr)
{
	struct run *r;

	// PGSIZE align 页的大小为0x1000,对某个页修改只需操作前4位，所以对齐
	addr = addr & ~(PGSIZE - 1);
	//页重新初始化
	memset((void *)addr, 0x0, (uint64)PGSIZE);
	//将更新后的页标记为free的
	r = (struct run *)addr;
	r->next = kmem.freelist;
	kmem.freelist = r;

	return;
}

/* 范围性free */
void kfreerange(char *start, char *end)
{
	char *addr = (char *)PGROUNDUP((uint64)start);
	for (; (uint64)(addr) + PGSIZE <= (uint64)end; addr += PGSIZE)
	{
		kfree((uint64)addr);
	}
}

/* 分配内存初始化，注意这里生成的页表集合是从高地址向低地址排序分布的
	已根据Lab4要求修改
*/
void mm_init(void)
{
	printk("...mm_init start\n");
	kfreerange((char *)_ekernel, (char *)(VM_START+PHY_SIZE)); //物理页表空间为kernel代码之后的PHY_SIZE大小的空间，_ekernel在vmlinux.lds文件中的末尾，标识end of kernel
	printk("...mm_init done!\n");
}
