#include <stdint.h>
#include <kernel.h>
#include <bitset.h>
#include <mm/pmm.h>

/* FIXME: bitset is huge - look into 4M pages or linked list ? */
static uint32_t frameset_data[8192]; /* 1GiB max memory */
/* Set bitset length to full size of data. Trim down to actual memsize later. */
static struct bitset frameset = BITSET_INIT(frameset_data, sizeof(frameset_data) * 32);

/* Allocates a single frame and returns the physical address */
phys_addr_t pmm_alloc()
{
	uint32_t frame_num;

	frame_num = bs_next_free(&frameset);

	if (frame_num == BITSET_NO_FREE)
		panic("OOM");

	bs_set(&frameset, frame_num);

	return FRAME_ADDR(frame_num);
}

void pmm_free(phys_addr_t paddr)
{
	bs_clear(&frameset, FRAME_NUM(paddr));
}

/* Initialises physical memory manager.
 *
 * Returns new end of kernel memory, after allocating required
 * data structures.
 */
phys_addr_t pmm_init(phys_addr_t mem_end)
{
	phys_addr_t addr;
	phys_addr_t kend;

	mem_end = PG_ALIGN_DOWN(mem_end);

	frameset.bitlen = PG_NUM(mem_end);

	kend = KERNEL_LD_PHYS_END;

	/* Pre-fill frame_set up to new kernel end address */
	for (addr = 0; addr < kend; addr += PG_SIZE)
		bs_set(&frameset, FRAME_NUM(addr));

	return kend;
}
