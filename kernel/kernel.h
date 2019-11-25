#ifndef __KERNEL_H_
#define __KERNEL_H_

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <config.h>
#include <mm/mmu.h>

#define ARRAY_SIZE(a) ((int)(sizeof(a) / sizeof(a[0])))

#define panic(str, ...) do {       \
	printf("PANIC: ");             \
	printf((str), ##__VA_ARGS__);  \
	while(1);                      \
} while(0);

extern uint32_t ticks;

#endif
