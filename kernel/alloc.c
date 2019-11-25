#include <alloc.h>
#include <mm/mmu.h>
#include <mm/pmm.h>
#include <mm/vmm.h>

void *heap_start;
void *heap_break;

#define MIN_SBRK_INCR (1 * PG_SIZE)

void *sbrk(intptr_t increment)
{
	void *brk = heap_break;

	if (increment == 0)
		goto out;

	if (increment < 0) {
		brk = (void *)~(size_t)0;
		goto out;
	}

	if (increment < MIN_SBRK_INCR)
		increment = MIN_SBRK_INCR;

	increment = PG_ALIGN(increment);

	/* TODO: Optimise by allocating only in page fault handler */
	while (increment) {
		phys_addr_t addr = pmm_alloc();
		vmm_map_pages(heap_break, 1, addr, 1, 0);
		increment -= PG_SIZE;
		heap_break += PG_SIZE;
	}

out:
	return brk;
}
