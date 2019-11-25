#include <stdint.h>

#include <mm/mmu.h>
#include <x86.h>

#define SEG(type, base, lim, dpl) (struct segdesc)          \
{                                                           \
	((lim) >> 12) & 0xffff, (uint32_t)(base) & 0xffff,      \
	((uint32_t)(base) >> 16) & 0xff, type, 1, dpl, 1,       \
	(uint32_t)(lim) >> 28, 0, 0, 1, 1, (uint32_t)(base) >> 24   \
}

#define SEG16(type, base, lim, dpl) (struct segdesc)        \
{                                                           \
	(lim) & 0xffff, (uint32_t)(base) & 0xffff,              \
	((uint32_t)(base) >> 16) & 0xff, type, 1, dpl, 1,       \
	(uint32_t)(lim) >> 16, 0, 0, 1, 0, (uint32_t)(base) >> 24   \
}

struct {
	uint16_t limit;
	uint32_t base;
} __attribute__((__packed__)) gdt_ptr;

extern void gdt_flush();

static struct segdesc gdt[SEG_NUM];
static struct tss_entry tss;

void gdtinit()
{
	gdt[SEG_NULL]  = SEG(0, 0, 0, 0);
	gdt[SEG_KCODE] = SEG(STA_X|STA_R, 0, 0xffffffff, 0);
	gdt[SEG_KDATA] = SEG(STA_W,       0, 0xffffffff, 0);
	gdt[SEG_UCODE] = SEG(STA_X|STA_R, 0, 0xffffffff, DPL_USER);
	gdt[SEG_UDATA] = SEG(STA_W,       0, 0xffffffff, DPL_USER);
	gdt[SEG_TSS]   = SEG16(STS_TG32, (uint32_t)&tss, sizeof(tss) - 1, 0);
	gdt[SEG_TSS].s = 0;

	tss.ss0 = SEG_KDATA << 3;
	tss.cs = (SEG_KCODE << 3) | DPL_USER;
	tss.ss = tss.ds = tss.es = tss.fs = tss.gs = (SEG_KDATA << 3) | DPL_USER;

	gdt_ptr.limit = sizeof(gdt) - 1;
	gdt_ptr.base = (uint32_t)&gdt[0];

	gdt_flush();
}

void set_kernel_stack(uint32_t sp)
{
	tss.esp0 = sp;
}
