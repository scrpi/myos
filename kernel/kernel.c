#include <kernel.h>
#include <mm/mmu.h>
#include <mm/pmm.h>
#include <mm/vmm.h>
#include <trap.h>
#include <drivers/tty.h>
#include <drivers/uart.h>
#include <multiboot.h>

uint32_t ticks;

void kernel_main(multiboot_info_t *mbt, uint32_t magic)
{
	phys_addr_t phys_end = 0;
	phys_addr_t kend = 0;

	terminal_initialize();
	uartinit();

	printf("Hello, kernel World!\n");
	printf("Magic: %x\n", magic);
	printf("KERNEL_LD_PHYS_START:%p\n", KERNEL_LD_PHYS_START);
	printf("KERNEL_LD_PHYS_END:%p\n", KERNEL_LD_PHYS_END);
	printf("KERNBASE:%p\n", KERNBASE);
	printf("KERNSTACK:%p\n", KERNSTACK);
	printf("KERNPD:%p\n", KERNPD);
	printf("KERNTOP:%p\n", KERNTOP);

	mbt = (multiboot_info_t *)P2V((uint8_t *)mbt);

	if (mbt->flags & MULTIBOOT_INFO_MEMORY) {
		printf("Lower:%d  Upper:%d\n", mbt->mem_lower, mbt->mem_upper);
		phys_end = mbt->mem_upper * 1024;
	}
	else
		printf("No multiboot memory info.\n");

	multiboot_memory_map_t *mmap = (multiboot_memory_map_t *)P2V(mbt->mmap_addr);
	while((void *)mmap < P2V(mbt->mmap_addr) + mbt->mmap_length) {
		printf("Memory map entry: addr=0x%llx\tlen=0x%llx\ttype=%s\n",
		       mmap->addr, mmap->len,
		       mmap->type == 1 ? "AVAILABLE" :
		       mmap->type == 2 ? "RESERVED" :
		       mmap->type == 3 ? "ACPI_RECLAIMABLE" :
		       mmap->type == 4 ? "NVS" : "BADRAM");

		if (mmap->type == 1 && mmap->addr + mmap->len > phys_end)
			phys_end = mmap->addr + mmap->len;

		mmap = (multiboot_memory_map_t*)((unsigned int)mmap + mmap->size + sizeof(mmap->size));
	}

	printf("Phys end: %dMiB\n", phys_end / 1024 / 1024);

	kend = pmm_init(phys_end);
	vmm_init(kend);
	gdtinit();
	trapinit();

	while(1);
}
