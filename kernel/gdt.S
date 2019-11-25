#include <mm/mmu.h>

.globl gdt_flush
.extern gdt_ptr
gdt_flush:
	lgdt gdt_ptr
	mov $(SEG_KDATA << 3), %ax
	mov %ax, %ds
	mov %ax, %es
	mov %ax, %fs
	mov %ax, %gs
	mov %ax, %ss
	mov %ax, %ds

	mov $(SEG_TSS << 3) & DPL_USER, %ax
	ltr %ax

	jmp $(SEG_KCODE << 3), $flush2

flush2:
	ret

