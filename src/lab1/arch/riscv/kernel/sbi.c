#include "types.h"
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
    // unimplemented    
	struct sbiret temp;
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
