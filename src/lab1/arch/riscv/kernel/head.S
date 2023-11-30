.extern start_kernel

    .section .text.entry
    .globl _start
_start:
    la sp,boot_stack_top
    tail start_kernel
    # ------------------
    # - your code here -
    # ------------------

    .section .bss.stack
    .globl boot_stack
boot_stack:
    .space 4096 # <-- change to your stack size

    .globl boot_stack_top
boot_stack_top: