#include <x86.h>

#define COM1 0x3f8

static int uart_present = 0;

void uartinit(void)
{
	/* Turn off the FIFO */
	outb(COM1 + 2, 0);

	/* 9600 baud, 8 data bits, 1 stop bit, parity off. */
	outb(COM1 + 3, 0x80); /* Unlock divisor */
	outb(COM1 + 0, 115200/9600);
	outb(COM1 + 1, 0);
	outb(COM1 + 3, 0x03); /* Lock divisor, 8 data bits. */
	outb(COM1 + 4, 0);
	outb(COM1 + 1, 0x01); /* Enable receive interrupts. */

	/* If status is 0xFF, no serial port. */
	if (inb(COM1 + 5) == 0xFF)
		return;

	uart_present = 1;

#if 0
	// Acknowledge pre-existing interrupt conditions;
	// enable interrupts.
	inb(COM1 + 2);
	inb(COM1 + 0);
	ioapicenable(IRQ_COM1, 0);
#endif
}

void uartputc(char c)
{
	int i;

	if (!uart_present)
		return;

	for (i = 0; i < 1000 && !(inb(COM1 + 5) & 0x20); ++i);

	outb(COM1 + 0, c);
}

char uartgetc(void)
{
	char ret;

	if (!uart_present)
		return -1;

	if (!(inb(COM1 + 5) & 0x01))
		return -1;

	ret = inb(COM1 + 0);

	return ret;
}
