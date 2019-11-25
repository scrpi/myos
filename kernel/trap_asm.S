#include <mm/mmu.h>

# vectors.S sends all traps here.
.globl trap_common
trap_common:
	# Build trap frame.
	pushl %ds
	pushl %es
	pushl %fs
	pushl %gs
	pushal

	# Set up data segments.
	movw $(SEG_KDATA<<3), %ax
	movw %ax, %ds
	movw %ax, %es

	# Call trap(tf), where tf=%esp
	pushl %esp
	call handle_trap
	addl $4, %esp

	popal
	popl %gs
	popl %fs
	popl %es
	popl %ds
	# trapno and errcode
	addl $0x8, %esp
	sti
	iret
