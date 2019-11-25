#ifndef __KERNEL_DRIVERS_UART_H_
#define __KERNEL_DRIVERS_UART_H_

#include "kernel.h"

/* Intel 8250 serial port (UART). */

void uartinit(void);
void uartputc(char c);
char uartgetc(void);

#endif
