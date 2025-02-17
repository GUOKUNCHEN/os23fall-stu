// clock.c
#include "clock.h"
#include "sbi.h"
// QEMU中时钟的频率是10MHz, 也就是1秒钟相当于10000000个时钟周期。

unsigned long TIMECLOCK = 10000000;

unsigned long get_cycles()
{
	// 编写内联汇编，使用 rdtime 获取 time 寄存器中 (也就是mtime 寄存器 )的值并返回
	// YOUR CODE HERE
	unsigned long __v;
	__asm__ volatile(
		"rdtime %[__v]\n"
		: [__v] "=r"(__v)
		:
		: "memory");
	return __v;
}

void clock_set_next_event()
{

	// 下一次 时钟中断 的时间点
	unsigned long next = get_cycles() + TIMECLOCK;

	// 使用 sbi_ecall 来完成对下一次时钟中断的设置
	// YOUR CODE HERE
	sbi_ecall(SBI_SETTIMER, 0, next, 0, 0, 0, 0, 0);
}