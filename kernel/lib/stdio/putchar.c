#include <stdio.h>

#include <drivers/tty.h>
#include <drivers/uart.h>

int putchar(int ic) {
	char c = (char) ic;
	terminal_write(&c, sizeof(c));
	uartputc(c);
	// TODO: Implement stdio and the write system call.
	return ic;
}
