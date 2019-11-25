#ifndef __KERNEL_PMM_H_
#define __KERNEL_PMM_H_

#include <mm/mmu.h>

phys_addr_t pmm_alloc();
void pmm_free(phys_addr_t paddr);

phys_addr_t pmm_init(phys_addr_t mem_end);

#endif
