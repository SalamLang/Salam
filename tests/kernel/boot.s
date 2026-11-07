/* Minimal Multiboot v1 loader for the i686 Salam kernel.
 * QEMU loads the ELF at 1 MiB and jumps here in 32-bit protected mode.
 */

.set MB_MAGIC, 0x1BADB002
.set MB_FLAGS, 0x00000003
.set MB_CHECK, -(MB_MAGIC + MB_FLAGS)

.section .multiboot, "a"
.align 4
.long MB_MAGIC
.long MB_FLAGS
.long MB_CHECK

.section .bss
.align 16
stack_bottom:
.skip 32768
stack_top:

.section .text
.code32
.global _start
.type _start, @function

_start:
    cli
    mov $stack_top, %esp

    mov %cr0, %eax
    and $0xFFFFFFFB, %eax
    or $0x2, %eax
    mov %eax, %cr0

    mov %cr4, %eax
    or $0x600, %eax
    mov %eax, %cr4
    fninit

    call main

1:
    cli
    hlt
    jmp 1b

.size _start, . - _start
