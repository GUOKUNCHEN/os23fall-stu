#ifndef _DEFS_H
#define _DEFS_H

#include "types.h"

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

#endif
