#include <mm/mmu.h>

# Declare constants for the multiboot header.
.set ALIGN,    1<<0             # align loaded modules on page boundaries
.set MEMINFO,  1<<1             # provide memory map
.set FLAGS,    ALIGN | MEMINFO  # this is the Multiboot 'flag' field
.set MAGIC,    0x1BADB002       # 'magic number' lets bootloader find the header
.set CHECKSUM, -(MAGIC + FLAGS) # checksum of above, to prove we are multiboot

# Declare a header as in the Multiboot Standard.
.section .multiboot
.align 4
.long MAGIC
.long FLAGS
.long CHECKSUM

# Allocate the initial stack.
.section .bootstrap_stack, "aw", @nobits
	.align 4096
.global _stack_page
_stack_page:
	.skip 4096 # 4 KiB

# Preallocate pages used for paging. Don't hard-code addresses and assume they
# are available, as the bootloader might have loaded its multiboot structures or
# modules there. This lets the bootloader know it must avoid the addresses.
.section .bss, "aw", @nobits
	.align 4096

.extern kernel_dir;

boot_page_table:
	.skip 4096

stack_page_table:
	.skip 4096

# Further page tables may be required if the kernel grows beyond 3 MiB.

# The kernel entry point.
.section .text
.global _start
.type _start, @function
_start:
	# Physical address of boot_page_table.
	# TODO: I recall seeing some assembly that used a macro to do the
	#       conversions to and from physical. Maybe this should be done in this
	#       code as well?
	movl $(boot_page_table - KERNBASE), %edi
	# First address to map is address 0.
	# TODO: Start at the first kernel page instead. Alternatively map the first
	#       1 MiB as it can be generally useful, and there's no need to
	#       specially map the VGA buffer.
	movl $0, %esi
	# Map 1023 pages. The 1024th will be the VGA text buffer.
	movl $1023, %ecx

1:
	# Map physical address as "present, writable". Note that this maps
	# .text and .rodata as writable. Mind security and map them as non-writable.
	movl %esi, %edx
	orl $0x003, %edx
	movl %edx, (%edi)

2:
	# Size of page is 4096 bytes.
	addl $4096, %esi
	# Size of entries in boot_page_table is 4 bytes.
	addl $4, %edi
	# Loop to the next entry if we haven't finished.
	loop 1b

3:
	# The page table is used at both page directory entry 0 (virtually from 0x0
	# to 0x3FFFFF) (thus identity mapping the kernel) and page directory entry
	# 768 (virtually from KERNBASE to 0xC03FFFFF) (thus mapping it in the
	# higher half). The kernel is identity mapped because enabling paging does
	# not change the next instruction, which continues to be physical. The CPU
	# would instead page fault if there was no identity mapping.

	# Map the page table to both virtual addresses 0x00000000 and KERNBASE.
	movl $(boot_page_table - KERNBASE + 0x003), kernel_dir - KERNBASE + 0
	movl $(boot_page_table - KERNBASE + 0x003), kernel_dir - KERNBASE + (KERNBASE >> 22) * 4

	# Map stack page PTE
	# This is getting ugly?
	movl $(_stack_page - KERNBASE + 0x003), stack_page_table - KERNBASE + (((KERNSTACK - 1) >> 12) % 1024) * 4

	# Map the page table containing the stack
	movl $(stack_page_table - KERNBASE + 0x003), kernel_dir - KERNBASE + ((KERNSTACK - 1) >> 22) * 4


	# Set cr3 to the address of kernel_dir.
	movl $(kernel_dir - KERNBASE), %ecx
	movl %ecx, %cr3

	# Enable paging and the write-protect bit.
	movl %cr0, %ecx
	orl $0x80010000, %ecx
	movl %ecx, %cr0

	# Jump to higher half with an absolute jump.
	lea 4f, %ecx
	jmp *%ecx

4:
	# At this point, paging is fully set up and enabled.

	# Unmap the identity mapping as it is now unnecessary.
	movl $0, kernel_dir + 0

	# Reload crc3 to force a TLB flush so the changes to take effect.
	movl %cr3, %ecx
	movl %ecx, %cr3

	# Set up the stack.
	movl $KERNSTACK, %esp

	# Push multiboot onto the stack
	pushl %eax # magic field
	pushl %ebx # multiboot_header

	# Transfer control to the main kernel.
	call kernel_main

	# Hang if kernel_main unexpectedly returns.
	cli
99:	hlt
	jmp 99b
.size _start, . - _start
