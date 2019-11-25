#include <stdint.h>

#include <kernel.h>
#include <x86.h>
#include <mm/mmu.h>
#include <trap.h>
#include <timer.h>
#include <drivers/uart.h>

#define PIC_MASTER_COMMAND  0x20
#define PIC_MASTER_DATA     0x21
#define PIC_MASTER_IMR      0x21
#define PIC_SLAVE_COMMAND   0xA0
#define PIC_SLAVE_DATA      0xA1
#define PIC_SLAVE_IMR       0xA1

/* System segment type bits */
#define STS_T32A    0x9     // Available 32-bit TSS
#define STS_IG32    0xE     // 32-bit Interrupt Gate
#define STS_TG32    0xF     // 32-bit Trap Gate

/*
 * Set up a normal interrupt/trap gate descriptor.
 * - istrap: 1 for a trap (= exception) gate, 0 for an interrupt gate.
 *   interrupt gate clears FL_IF, trap gate leaves FL_IF alone
 * - sel: Code segment selector for interrupt/trap handler
 * - off: Offset in code segment for interrupt/trap handler
 * - dpl: Descriptor Privilege Level -
 *        the privilege level required for software to invoke
 *        this interrupt/trap gate explicitly using an int instruction.
 */
#define SETGATE(gate, istrap, sel, off, d)           \
{                                                    \
  (gate).off_15_0 = (uint32_t)(off) & 0xffff;        \
  (gate).cs = (sel);                                 \
  (gate).args = 0;                                   \
  (gate).rsv1 = 0;                                   \
  (gate).type = (istrap) ? STS_TG32 : STS_IG32;      \
  (gate).s = 0;                                      \
  (gate).dpl = (d);                                  \
  (gate).p = 1;                                      \
  (gate).off_31_16 = (uint32_t)(off) >> 16;          \
}

/* Layout of the trap frame built on the stack by the
 * hardware and by trapasm.S, and passed to trap().
 */
struct trapframe {
	/* registers as pushed by pusha */
	uint32_t edi;
	uint32_t esi;
	uint32_t ebp;
	uint32_t oesp;
	uint32_t ebx;
	uint32_t edx;
	uint32_t ecx;
	uint32_t eax;

	/* rest of trap frame */
	uint16_t gs;
	uint16_t padding1;
	uint16_t fs;
	uint16_t padding2;
	uint16_t es;
	uint16_t padding3;
	uint16_t ds;
	uint16_t padding4;
	uint32_t trapno;

	/* below here defined by x86 hardware */
	uint32_t err;
	uint32_t eip;
	uint16_t cs;
	uint16_t padding5;
	uint32_t eflags;

	/* below here only when crossing rings, such as from user to kernel */
	uint32_t esp;
	uint16_t ss;
	uint16_t padding6;
};

struct gatedesc idt[256];

extern uint32_t vectors[]; /* in vectors.S */

void pic_remap()
{
	outb(PIC_MASTER_COMMAND, 0x11 );
	outb(PIC_SLAVE_COMMAND, 0x11 );
	outb(PIC_MASTER_DATA, T_IRQ0);
	outb(PIC_SLAVE_DATA, T_IRQ0 + 8 );
	outb(PIC_MASTER_DATA, 0x04);
	outb(PIC_SLAVE_DATA, 2);
	outb(PIC_MASTER_DATA, 0x01);
	outb(PIC_SLAVE_DATA, 0x01);
}

void pic_eoi(int irq_num)
{
	outb(PIC_MASTER_COMMAND, 0x20);
	if (irq_num > 7)
		outb(PIC_SLAVE_COMMAND, 0x20);
}

void trapinit()
{
	int i;

	for (i = 0; i < ARRAY_SIZE(idt); ++i)
		SETGATE(idt[i], 0, SEG_KCODE << 3, vectors[i], 0);

	SETGATE(idt[T_SYSCALL], 1, SEG_KCODE << 3, vectors[T_SYSCALL], DPL_USER);

	lidt(idt, sizeof(idt));

	pic_remap();

	init_timer(CONFIG_HZ);

	__asm__("sti");
}

void do_page_fault(struct trapframe *tf)
{
	uint32_t faulting_address;

	__asm__ volatile("mov %%cr2, %0" : "=r" (faulting_address));

	printf("Page Fault! (");

	printf("present=%d, ",  !!(tf->err & 0x1));
	printf("read=%d, ",      !(tf->err & 0x2));
	printf("user=%d, ",     !!(tf->err & 0x4));
	printf("reserved=%d, ", !!(tf->err & 0x8));
	printf("id=%d",         !!(tf->err & 0x10));

	printf(") at 0x%x\n", faulting_address);

	panic("PANIC\n");
}

void handle_trap(struct trapframe *tf)
{
	uint32_t tmp;

	switch (tf->trapno) {
	case T_IRQ0 + IRQ_TIMER:
		timer_isr();
		pic_eoi(tf->trapno);
		break;

	case T_IRQ0 + IRQ_COM1:
		printf(" COM1!! (ticks: %d)\n", ticks);
		printf("%c", uartgetc());
		pic_eoi(tf->trapno);
		break;

	case T_IRQ0 + IRQ_KBD:
		printf(" KBD!! (ticks: %d)\n", ticks);
		tmp = inb(0x64);
		tmp = inb(0x60);
		pic_eoi(tf->trapno);
		break;

	case T_PGFLT:
		do_page_fault(tf);
		break;

	default:
		printf("unexpected trap=%d eip=0x%x error=0x%x\n", tf->trapno, tf->eip, tf->err);
		panic("Unexpected Trap\n");
		break;
	}

	(void)tmp;
}
