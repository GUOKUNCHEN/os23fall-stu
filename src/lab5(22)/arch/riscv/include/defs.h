#ifndef _DEFS_H
#define _DEFS_H

//#include "types.h"

#define csr_read(csr)                  \
	({                                 \
		register uint64 __v;           \
		/* unimplemented */            \
		asm volatile("csrr %0," #csr   \
					 : "=r"(__v)       \
					 :                 \
					 : "memory");      \
		/* 从寄存器中获取值 */ \
		__v;                     \
	})

#define csr_write(csr, val)                         \
({                                                  \
    uint64 __v = (uint64)(val);                     \
    asm volatile ("csrw " #csr ", %0"               \
                    : : "r" (__v)                   \
                    : "memory");                    \
})

// Lab3添加部分
#define PHY_START 0x0000000080000000
#define PHY_SIZE  128 * 1024 * 1024 // 128MB,  QEMU 默认内存大小
#define PHY_END   (PHY_START + PHY_SIZE)

#define PGSIZE 0x1000 // 4KB
#define PGROUNDUP(addr) ((addr + PGSIZE - 1) & (~(PGSIZE - 1)))
#define PGROUNDDOWN(addr) (addr & (~(PGSIZE - 1)))

// Lab4添加部分
#define OPENSBI_SIZE (0x200000)

#define VM_START (0xffffffe000000000)	//虚拟地址头
#define VM_END   (0xffffffff00000000)	//虚拟地址尾
#define VM_SIZE  (VM_END - VM_START)

#define PA2VA_OFFSET (VM_START - PHY_START)

//Lab4个人添加内容（权限宏定义）
#define PTE_V 0x1
#define PTE_R 0x2
#define PTE_W 0x4
#define PTE_X 0x8
#define PTE_U 16


// Lab5添加部分

#define USER_START (0x0000000000000000) // user space start virtual address
#define USER_END   (0x0000004000000000) // user space end virtual address



#endif
