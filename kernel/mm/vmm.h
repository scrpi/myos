#ifndef __KERNEL_VMM_H_
#define __KERNEL_VMM_H_

#include <mm/mmu.h>

struct pte {
	uint32_t present   : 1;  /* Page present in memory */
	uint32_t rw        : 1;  /* Read-only if clear, read-write if set */
	uint32_t user      : 1;  /* Supervisor level only if clear */
	uint32_t accessed  : 1;  /* Has the page been accessed since last refresh ? */
	uint32_t dirty     : 1;  /* Has the page been written to since last refresn ? */
	uint32_t res       : 7;  /* Amalgamation of unused and reserved bits */
	uint32_t frame     : 20; /* Frame address (shifted right 12 bits) */
};

struct pde {
	struct pte pages[1024];
};

struct pd {
	phys_addr_t tables[1024];
};

struct pd *vmm_create_pd(phys_addr_t paddr);
void vmm_switch_to_user();
void vmm_switch_to_kern();
void vmm_switch_dir(phys_addr_t pd_dir);
int vmm_map_pages(void *vaddr, size_t num, phys_addr_t paddr, int rw, int user);

void vmm_init(phys_addr_t kend);


#endif
