    .section .text.entry
    .align 2
    .globl _traps 
_traps:
    csrrw sp,sscratch,sp
	bne sp, x0, _traps_switch
	csrrw sp,sscratch,sp
_traps_switch:
    # save 32 registers and sepc to stack
	sd x2,-32*8(sp)
   	addi sp,sp,-34*8
	sd x0,0*8(sp)
	sd x1,1*8(sp)
	sd x3,3*8(sp)
	sd x4,4*8(sp)
	sd x5,5*8(sp)
	sd x6,6*8(sp)
	sd x7,7*8(sp)
	sd x8,8*8(sp)
	sd x9,9*8(sp)
	sd x10,10*8(sp)
	sd x11,11*8(sp)
	sd x12,12*8(sp)
	sd x13,13*8(sp)
	sd x14,14*8(sp)
	sd x15,15*8(sp)
	sd x16,16*8(sp)
	sd x17,17*8(sp)
	sd x18,18*8(sp)
	sd x19,19*8(sp)
	sd x20,20*8(sp)
	sd x21,21*8(sp)
	sd x22,22*8(sp)
	sd x23,23*8(sp)
	sd x24,24*8(sp)
	sd x25,25*8(sp)
	sd x26,26*8(sp)
	sd x27,27*8(sp)
	sd x28,28*8(sp)
	sd x29,29*8(sp)
	sd x30,30*8(sp)
	sd x31,31*8(sp)
	csrr t0,sepc
	sd t0,32*8(sp)
	csrr t0,sstatus
	sd t0,33*8(sp)

    # call trap_handler
    csrr a0,scause
	csrr a1,sepc
	mv a2,sp #前34段内容对应34个寄存器，符合pt_regs结构体定义，方便调用
	call ra,trap_handler

    # restore sepc and 32 registers (x2(sp) should be restore last) from stack
    #先用t0加载sepc和sstatus后,再恢复t0(x5)
	ld t0,32*8(sp)
	addi t0,t0,4	#手动+4
	csrw sepc,t0
	ld t0,33*8(sp)
	csrw sstatus,t0
	
	ld x1,1*8(sp)	#x0寄存器无需恢复
	ld x3,3*8(sp)
	ld x4,4*8(sp)
	ld x5,5*8(sp)
	ld x6,6*8(sp)
	ld x7,7*8(sp)
	ld x8,8*8(sp)
	ld x9,9*8(sp)
	ld x10,10*8(sp)
	ld x11,11*8(sp)
	ld x12,12*8(sp)
	ld x13,13*8(sp)
	ld x14,14*8(sp)
	ld x15,15*8(sp)
	ld x16,16*8(sp)
	ld x17,17*8(sp)
	ld x18,18*8(sp)
	ld x19,19*8(sp)
	ld x20,20*8(sp)
	ld x21,21*8(sp)
	ld x22,22*8(sp)
	ld x23,23*8(sp)
	ld x24,24*8(sp)
	ld x25,25*8(sp)
	ld x26,26*8(sp)
	ld x27,27*8(sp)
	ld x28,28*8(sp)
	ld x29,29*8(sp)
	ld x30,30*8(sp)
	ld x31,31*8(sp)
	ld sp,2*8(sp)#恢复栈指针sp(x2)

   	csrrw sp,sscratch,sp
	bne sp, x0, _traps_end
	csrrw sp,sscratch,sp
_traps_end:
	sret

    .globl __switch_to
__switch_to:
    # save state to prev process
    sd ra,5*8(a0)
	sd sp,6*8(a0)
	sd s0,7*8(a0)
	sd s1,8*8(a0)
	sd s2,9*8(a0)
	sd s3,10*8(a0)
	sd s4,11*8(a0)
	sd s5,12*8(a0)
	sd s6,13*8(a0)
	sd s7,14*8(a0)
	sd s8,15*8(a0)
	sd s9,16*8(a0)
	sd s10,17*8(a0)
	sd s11,18*8(a0)

	#Lab5添加部分，保存sepc,sstatus,sscratch和satp
	csrr t0,sepc
	sd t0,19*8(a0)
	csrr t0,sstatus
	sd t0,20*8(a0)
	csrr t0,sscratch
	sd t0,21*8(a0)
	csrr t0, satp
	sd t0, 22*8(a0)

    # restore state from next process
    # YOUR CODE HERE
	ld ra,5*8(a1)
	ld sp,6*8(a1)
	ld s0,7*8(a1)
	ld s1,8*8(a1)
	ld s2,9*8(a1)
	ld s3,10*8(a1)
	ld s4,11*8(a1)
	ld s5,12*8(a1)
	ld s6,13*8(a1)
	ld s7,14*8(a1)
	ld s8,15*8(a1)
	ld s9,16*8(a1)
	ld s10,17*8(a1)
	ld s11,18*8(a1)

	#Lab5添加部分，恢复sepc,sstatus,sscratch和satp,同时刷缓存进行页表切换
	ld t0,19*8(a1)
	csrw sepc,t0
	ld t0,20*8(a1)
	csrw sstatus,t0
	ld t0,21*8(a1)
	csrw sscratch,t0
	ld t0, 22*8(a1)
	csrw satp,t0

	# flush tlb
	sfence.vma zero, zero

    ret

    .globl __dummy
__dummy:
	#Lab5内容,在RISC-V架构下，我们需要将U态和S态的栈指针进行交换，实现栈切换,此时切换到U态
	csrrw sp,sscratch,sp
	bne sp, x0, __dummy_end
	csrrw sp,sscratch,sp
__dummy_end:
	sret