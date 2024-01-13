#include"defs.h"
.extern start_kernel
.extern mm_init		#lab3 uses for process schedule init
.extern task_init	#lab3 uses for process schedule init
.extern early_pgtbl	#lab4 uses for vm_locating

    .section .text.init
    .globl _start
_start:
    # YOUR CODE HERE
	la sp,boot_stack_top
	# li t0, PA2VA_OFFSET
	# neg t0,t0
	# add sp,sp,t0

	# ------------------
		# Lab4 adds.
	call setup_vm	# init vm before context display.
    call relocate	# set satp to enable Sv39 mode vm mapping.

_end_relocate:
	call ra,mm_init
	call setup_vm_final

	call ra,task_init	# task[i]'s PC start place is included in "task_init" 
    # ------------------
        # set stvec = _traps (Lab2)
	
	la a0,_traps
	csrw stvec,a0

    # ------------------

        # set sie[STIE] = 1
		# position 1 set 1
	
	andi a0,a0,0
	addi a0,a0,32 
	csrw sie,a0


    # ------------------

        # set sstatus[SIE] = 1

    # ------------------

	#csrsi sstatus,(1<<1)

    # ------------------
    	# --- lab1 code ----
	j start_kernel

relocate:
	# ------------------

		#  Lab4 Codes,here we need to set 'satp'

    	# set ra = ra + PA2VA_OFFSET
    	# set sp = sp + PA2VA_OFFSET (If you have set the sp before)

	li t0, PA2VA_OFFSET
	add ra,ra,t0
	add sp,sp,t0
	
    ###################### 
    #   YOUR CODE HERE   #
    ######################

    # set satp with early_pgtbl
    li t0, 8
    slli t0, t0, 60
    la t1, early_pgtbl
    srli t1, t1, 12
    add t0, t0, t1
    csrw satp, t0
    
    # flush tlb
    sfence.vma zero, zero

    ret

    .section .bss.stack
    .globl start_kernel
boot_stack:
    .space 4096 # <-- change to your stack size

    .globl boot_stack_top
boot_stack_top: