/* 将 ext (Extension ID) 放入寄存器 a7 中，
fid (Function ID) 放入寄存器 a6 中，
将 arg0 ~ arg5 放入寄存器 a0 ~ a5 中。
使用 ecall 指令。ecall 之后系统会进入 M 模式，之后 OpenSBI 会完成相关操作。
OpenSBI 的返回结果会存放在寄存器 a0 ， a1 中，其中 a0 为 error code， a1 为返回值， 我们用 sbiret 来接受这两个返回值。 */
#include "sbi.h"

#define write_to_reg(reg, var)       \
	__asm__ volatile(                \
		"mv " #reg ", %[" #var "]\n" \
		:                            \
		: [var] "r"(var)             \
		: "memory")

struct sbiret sbi_ecall(int ext, int fid, uint64 arg0,
						uint64 arg1, uint64 arg2,
						uint64 arg3, uint64 arg4,
						uint64 arg5)
{
	struct sbiret temp;
	//把信息写入寄存器
	// unimplemented
	/* __asm__ volatile(
		"mv a7,%[ext]\n"
		"mv a6,%[fid]\n"
		"mv a0,%[arg0]\n"
		"mv a1,%[arg1]\n"
		"mv a2,%[arg2]\n"
		"mv a3,%[arg3]\n"
		"mv a4,%[arg4]\n"
		"mv a5,%[arg5]\n"
		:
		: [ext] "r"(ext),
		  [fid] "r"(fid),
		  [arg0] "r"(arg0),
		  [arg1] "r"(arg1),
		  [arg2] "r"(arg2),
		  [arg3] "r"(arg3),
		  [arg4] "r"(arg4),
		  [arg5] "r"(arg5)
		  ); */
	write_to_reg(a7, ext);
	write_to_reg(a6, fid);
	write_to_reg(a0, arg0);
	write_to_reg(a1, arg1);
	write_to_reg(a2, arg2);
	write_to_reg(a3, arg3);
	write_to_reg(a4, arg4);
	write_to_reg(a5, arg5);
	//调用指令ecall
	__asm__ volatile(
		"ecall\n");

	//接收SBI返回的结果
	__asm__ volatile(
		"mv %[error], a0\n"
		"mv %[value], a1"
		:[error]"=r"(temp.error),[value]"=r"(temp.value)
		::"memory"
	);
	return temp;
}
