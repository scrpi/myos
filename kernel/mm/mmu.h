#ifndef __KERNEL_MMU_H_
#define __KERNEL_MMU_H_

#define P2V(addr) ((void *)((addr) + KERNBASE))
#define V2P(addr) ((phys_addr_t)(addr) - KERNBASE)

#define KERNBASE           0xC0000000

/* By default paging_init maps from KERNBASE -> (KERNTOP - 1) */
#define KERNTOP            0xF0000000

/* Number of reserved pages at top of mem for allocating page directories */
#define KERNPDPAGES        4096

#define KERNPD             (KERNTOP - (KERNPDPAGES * PG_SIZE))

#define KERNSTACK          KERNPD
#define KERNSTACKPAGES     ((8 * 1024 * 1024) / PG_SIZE) /* 8MiB */

#define KERNEL_LD_VIRT_START ((phys_addr_t)&_kernel_start)
#define KERNEL_LD_VIRT_END   ((phys_addr_t)&_kernel_end)
#define KERNEL_LD_PHYS_START V2P(KERNEL_LD_VIRT_START)
#define KERNEL_LD_PHYS_END   V2P(KERNEL_LD_VIRT_END)

#define PG_SIZE       4096 /* Page size in bytes */
#define PG_SHIFT      12
#define PG_NUM(addr)  ((uint32_t)(addr) >> PG_SHIFT)
#define PG_ADDR(num)  ((void *)((num)  << PG_SHIFT))

#define PG_ALIGN_DOWN(a) ((a) & ~(PG_SIZE - 1))
#define PG_ALIGN(a)      PG_ALIGN_DOWN((a) + PG_SIZE - 1)

#define FRAME_NUM(addr)  PG_NUM((uint32_t)addr)
#define FRAME_ADDR(num)  ((phys_addr_t)PG_ADDR(num))

#define PDE_SHIFT         10
#define PDE_IDX(addr)     (PG_NUM(addr) >> PDE_SHIFT)
#define PDE_OFFSET(addr)  (PG_NUM(addr) & ((1 << PDE_SHIFT) - 1))

#define CURRENT_PD_PTR             ((struct pd *)0xFFFFF000)
#define CURRENT_PD_PDE_PTR(index)  ((struct pde *)(0xFFC00000 + (index) * PG_SIZE))

#define PTE_P  (0x01 << 0)
#define PTE_W  (0x01 << 1)
#define PTE_U  (0x01 << 2)

/* Application segment type bits */
#define STA_X  0x8 /* Executable segment */
#define STA_W  0x2 /* Writeable (non-executable segments) */
#define STA_R  0x2 /* Readable (executable segments) */

/* System segment type bits */
#define STS_T32A  0x9 /* Available 32-bit TSS */
#define STS_IG32  0xE /* 32-bit Interrupt Gate */
#define STS_TG32  0xF /* 32-bit Trap Gate */

/* User DPL */
#define DPL_USER  0x3

#define SEG_NULL  0
#define SEG_KCODE 1
#define SEG_KDATA 2
#define SEG_UCODE 3
#define SEG_UDATA 4
#define SEG_TSS   5

#define SEG_NUM 6

#ifndef __ASSEMBLER__

#include <stdint.h>

typedef uint32_t phys_addr_t;

extern uint32_t _kernel_start;
extern uint32_t _kernel_end;

struct segdesc {
	uint32_t lim_15_0 : 16;  /* Low bits of segment limit */
	uint32_t base_15_0 : 16; /* Low bits of segment base address */
	uint32_t base_23_16 : 8; /* Middle bits of segment base address */
	uint32_t type : 4;       /* Segment type (see STS_ constants) */
	uint32_t s : 1;          /* 0 = system, 1 = application */
	uint32_t dpl : 2;        /* Descriptor Privilege Level */
	uint32_t p : 1;          /* Present */
	uint32_t lim_19_16 : 4;  /* High bits of segment limit */
	uint32_t avl : 1;        /* Unused (available for software use) */
	uint32_t rsv1 : 1;       /* Reserved */
	uint32_t db : 1;         /* 0 = 16-bit segment, 1 = 32-bit segment */
	uint32_t g : 1;          /* Granularity: limit scaled by 4K when set */
	uint32_t base_31_24 : 8; /* High bits of segment base address */
};

struct tss_entry {
	uint32_t prev_tss;   /* The previous TSS - if we used hardware task switching this would form a linked list. */
	uint32_t esp0;       /* The stack pointer to load when we change to kernel mode. */
	uint32_t ss0;        /* The stack segment to load when we change to kernel mode. */
	uint32_t esp1;       /* Unused... */
	uint32_t ss1;
	uint32_t esp2;
	uint32_t ss2;
	uint32_t cr3;
	uint32_t eip;
	uint32_t eflags;
	uint32_t eax;
	uint32_t ecx;
	uint32_t edx;
	uint32_t ebx;
	uint32_t esp;
	uint32_t ebp;
	uint32_t esi;
	uint32_t edi;
	uint32_t es;         /* The value to load into ES when we change to kernel mode. */
	uint32_t cs;         /* The value to load into CS when we change to kernel mode. */
	uint32_t ss;         /* The value to load into SS when we change to kernel mode. */
	uint32_t ds;         /* The value to load into DS when we change to kernel mode. */
	uint32_t fs;         /* The value to load into FS when we change to kernel mode. */
	uint32_t gs;         /* The value to load into GS when we change to kernel mode. */
	uint32_t ldt;        /* Unused... */
	uint16_t trap;
	uint16_t iomap_base;
} __attribute__((__packed__));

void gdtinit();
void set_kernel_stack(uint32_t sp);

#endif

#endif
