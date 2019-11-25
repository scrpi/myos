#include <kernel.h>
#include <bitset.h>
#include <mm/vmm.h>
#include <mm/pmm.h>
#include <alloc.h>

__attribute((__aligned__(PG_SIZE)))
struct pd kernel_dir;

extern phys_addr_t _stack_page;

static uint32_t pdset_data[KERNPDPAGES / 32];
static struct bitset pdset = BITSET_INIT(pdset_data, KERNPDPAGES);

static inline void self_map(struct pd *dir, phys_addr_t paddr)
{
	dir->tables[1023] = paddr | PTE_P | PTE_W;
}

/* Gets a page pointer via recursive self-mapping.
 *
 * Requires that the directory of interest is currently mapped
 */
static struct pte *get_pte_from_current(void *vaddr, int create)
{
	phys_addr_t addr;
	struct pde *table;
	struct pd *dir;
	uint32_t idx;

	idx = PDE_IDX(vaddr);

	/* Point dir to self-mapped page directory */
	dir = CURRENT_PD_PTR;

	/* Point table to self-mapped PDE for idx */
	table = CURRENT_PD_PDE_PTR(idx);

	/* Determine if a pde (page table) is already present */
	if (dir->tables[idx] & PTE_P)
		goto out;

	/* Page table not allocated yet */
	if (!create)
		return NULL;

	addr = pmm_alloc();
	dir->tables[idx] = addr | PTE_P | PTE_W;
	/* TODO: Invalidate TLB now? */
	memset(table, 0, sizeof(*table));

out:
	return &table->pages[PDE_OFFSET(vaddr)];
}

static phys_addr_t init_kern_tables(phys_addr_t kend)
{
	phys_addr_t alloc_end = 0;
	uint32_t frame_num = 0;
	int table_count;
	uint32_t vmemsz;
	int i, j;

	vmemsz = KERNTOP - KERNBASE;
	vmemsz = PG_ALIGN_DOWN(vmemsz);
	printf("vmemsz: 0x%x\n", vmemsz);

	table_count = PG_NUM(vmemsz) >> PDE_SHIFT;

	self_map(&kernel_dir, V2P(&kernel_dir));

	for (i = 0; i < table_count; ++i) {
		struct pde *pde;
		uint32_t idx;

		idx = PDE_IDX(KERNBASE) + i;

		if (!(kernel_dir.tables[idx] & PTE_P)) {
			phys_addr_t table_addr;

			/* PDE not already present */
			table_addr = pmm_alloc();

			if (table_addr > alloc_end)
				alloc_end = table_addr;

			kernel_dir.tables[idx] = table_addr | PTE_P | PTE_W;

			pde = P2V(table_addr);
			memset(pde, 0, sizeof(*pde));
		}
		else {
			pde = P2V(PG_ALIGN_DOWN(kernel_dir.tables[idx]));
		}

		/* Skip mapping physical kernel pages if we've already
		 * mapped them all in the loop below.
		 */
		if (frame_num > FRAME_NUM(kend))
			continue;

		for (j = 0; j < 1024; ++j) {
			frame_num = i * 1024 + j;

			/* Only physically map fromn 0x0 -> kend */
			if (frame_num > FRAME_NUM(kend))
				break;

			struct pte *p = &pde->pages[j];

			if (p->present)
				continue;

			p->frame = frame_num;
			p->present = 1;
			p->rw = 1;
			p->user = 0;
		}
	}

	return PG_ALIGN(alloc_end + PG_SIZE);
}

static phys_addr_t map_initial_stack_pages(phys_addr_t kend)
{
	phys_addr_t f;
	void *vaddr;
	int i;

	/* First page is already mapped */
	vaddr = (void *)PG_ALIGN_DOWN(KERNSTACK - 1);
	i = 1;

	for (; i < KERNSTACKPAGES; ++i) {
		vaddr = (void *)PG_ALIGN_DOWN((uint32_t)vaddr - 1);
		f = pmm_alloc();

		vmm_map_pages(vaddr, 1, f, 1, 0);

		if (f > kend)
			kend = f;
	}

	return PG_ALIGN(kend + PG_SIZE);
}

struct pd *vmm_create_pd(phys_addr_t paddr)
{
	uint32_t vaddr_idx;
	struct pd *pd;
	int i;

	vaddr_idx = bs_next_free(&pdset);
	if (vaddr_idx == BITSET_NO_FREE)
		panic("%s: no memory available for new page directory\n", __func__);

	pd = (struct pd *)(KERNPD + (vaddr_idx * PG_SIZE));
	vmm_map_pages(pd, 1, paddr, 1, 0);

	/* Copy kernel higher half */
	for (i = PDE_IDX(KERNBASE); i < 1024; ++i)
		pd->tables[i] = kernel_dir.tables[i];

	return pd;
}

void vmm_switch_dir(phys_addr_t pd_dir)
{
	uint32_t cr0;

	printf("cr3 <== %x\n", pd_dir);
	__asm__ volatile("mov %0, %%cr3" : : "r" (pd_dir));
	__asm__ volatile("mov %%cr0, %0" : "=r" (cr0));
	cr0 |= 0x80000000;
	__asm__ volatile("mov %0, %%cr0" : : "r" (cr0));
}

/* Map consecutive 'num' set of pages to consecutive set of frames
 *
 * Requires that the directory of interest is currently mapped
 */
int vmm_map_pages(void *vaddr, size_t num, phys_addr_t paddr, int rw, int user)
{
	struct pte *p;

	/* Force page alignment of virtual address */
	vaddr = (void *)PG_ALIGN_DOWN((uint32_t)vaddr);

	while (num--) {
		p = get_pte_from_current(vaddr, 1);

		if (!p || p->present)
			return -1;

		p->frame = PG_ALIGN_DOWN(paddr) >> PG_SHIFT;
		p->present = 1;
		p->rw = rw;
		p->user = user;

		vaddr += PG_SIZE;
		paddr += PG_SIZE;
	}

	return 0;
}

void vmm_init(phys_addr_t kend)
{
	/* Map from KERNBASE -> (KERNTOP - 1) */
	kend = init_kern_tables(kend);
	kend = map_initial_stack_pages(kend);

	/* Set up for malloc/sbrk */
	heap_start = heap_break = P2V(PG_ALIGN(kend));
	printf("Heap start: 0x%x\n", heap_start);

	vmm_switch_dir(V2P(&kernel_dir));
}
