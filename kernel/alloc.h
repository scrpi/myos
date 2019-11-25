#ifndef __KERNEL_ALLOC_H_
#define __KERNEL_ALLOC_H_

#include "kernel.h"

extern void *heap_start;
extern void *heap_break;

void *sbrk(intptr_t increment);


#endif
