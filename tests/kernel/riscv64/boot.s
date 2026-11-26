/* Entry point for the riscv64 Salam kernel on the QEMU virt board.
 * QEMU starts every core at 0x80000000 in machine mode (-bios none).
 * Core 0 clears .bss, turns on the FPU, sets up a stack and calls the Salam main;
 * the other cores wait forever.
 */

.section .text.boot, "ax"
.global _start
.type _start, @function

_start:
    csrr t0, mhartid
    bnez t0, 2f

.option push
.option norelax
    la gp, __global_pointer$
.option pop
    la sp, stack_top

    la t0, __bss_start
    la t1, __bss_end
1:
    bgeu t0, t1, 3f
    sd zero, 0(t0)
    addi t0, t0, 8
    j 1b

3:
    li t0, 0x2000
    csrs mstatus, t0
    csrw fcsr, zero
    call main

2:
    wfi
    j 2b

.size _start, . - _start

.section .bss
.align 16
stack_bottom:
.skip 65536
stack_top:
