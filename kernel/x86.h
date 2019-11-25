#ifndef __KERNEL_X86_H_
#define __KERNEL_X86_H_

#include <stdint.h>

struct segdesc;
struct gatedesc;

__attribute__((unused))
static inline uint8_t outb(uint16_t port, uint8_t value)
{
	__asm__ volatile ("outb %1, %0" : : "dN" (port), "a" (value));
	return value;
}

__attribute__((unused))
static inline uint16_t outw(uint16_t port, uint16_t value)
{
	__asm__ volatile ("outw %1, %0" : : "dN" (port), "a" (value));
	return value;
}

__attribute__((unused))
static inline uint32_t outl(uint16_t port, uint32_t value)
{
	__asm__ volatile ("outl %1, %0" : : "dN" (port), "a" (value));
	return value;
}

__attribute__((unused))
static inline uint8_t inb(uint16_t port)
{
	uint8_t result;
	__asm__ volatile("inb %1, %0" : "=a" (result) : "dN" (port));
	return result;
}

__attribute__((unused))
static inline uint16_t inw(uint16_t port)
{
	uint16_t result;
	__asm__ volatile("inw %1, %0" : "=a" (result) : "dN" (port));
	return result;
}

__attribute__((unused))
static inline uint32_t inl(uint16_t port)
{
	uint32_t result;
	__asm__ volatile("inl %1, %0" : "=a" (result) : "dN" (port));
	return result;
}

static inline void lgdt(struct segdesc *p, int size)
{
	volatile uint16_t pd[3];

	pd[0] = size-1;
	pd[1] = (uint32_t)p;
	pd[2] = (uint32_t)p >> 16;

	__asm__ volatile("lgdt (%0)" : : "r" (pd));
}

static inline void lidt(struct gatedesc *p, int size)
{
	volatile uint16_t pd[3];

	pd[0] = size-1;
	pd[1] = (uint32_t)p;
	pd[2] = (uint32_t)p >> 16;

	__asm__ volatile("lidt (%0)" : : "r" (pd));
}

#endif
