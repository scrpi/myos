#ifndef __KERNEL_TIMER_H_
#define __KERNEL_TIMER_H_

#include <stdint.h>

void init_timer(uint32_t freq);
void timer_isr();

#endif
