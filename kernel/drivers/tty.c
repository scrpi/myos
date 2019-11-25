#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>

#include <kernel.h>
#include <drivers/tty.h>

#include "vga.h"

#define VGA_ADDR    (KERNBASE + 0xb8000)
#define VGA_WIDTH   80
#define VGA_HEIGHT  25
#define VGA_TOP     (VGA_ADDR + (2 * VGA_WIDTH * VGA_HEIGHT))

static uint16_t* const VGA_MEMORY = (uint16_t*)VGA_ADDR;

static size_t terminal_row;
static size_t terminal_column;
static uint8_t terminal_color;
static uint16_t* terminal_buffer;

void terminal_initialize(void) {
	terminal_row = 0;
	terminal_column = 0;
	terminal_color = vga_entry_color(VGA_COLOR_LIGHT_GREY, VGA_COLOR_BLACK);
	terminal_buffer = VGA_MEMORY;
	for (size_t y = 0; y < VGA_HEIGHT; y++) {
		for (size_t x = 0; x < VGA_WIDTH; x++) {
			const size_t index = y * VGA_WIDTH + x;
			terminal_buffer[index] = vga_entry(' ', terminal_color);
		}
	}
}

void terminal_setcolor(uint8_t color) {
	terminal_color = color;
}

void terminal_putentryat(unsigned char c, uint8_t color, size_t x, size_t y) {
	const size_t index = y * VGA_WIDTH + x;
	terminal_buffer[index] = vga_entry(c, color);
}

static void scrollup()
{
	uint8_t blank_color;
	uint16_t *w, *r;
	int i;

	w = VGA_MEMORY;
	r = VGA_MEMORY + VGA_WIDTH;

	while (r < (uint16_t *)VGA_TOP) {
		*w++ = *r++;
	}

	blank_color = vga_entry_color(VGA_COLOR_LIGHT_GREY, VGA_COLOR_BLACK);

	for (i = 0; i < VGA_WIDTH; ++i)
		terminal_putentryat(' ', blank_color, i, VGA_HEIGHT - 1);
}

void terminal_putchar(char c)
{
	unsigned char uc = c;

	if (c == '\n') {
		terminal_column = 0;
		if (++terminal_row == VGA_HEIGHT) {
			scrollup();
			terminal_row = VGA_HEIGHT - 1;
		}
		return;
	}

	terminal_putentryat(uc, terminal_color, terminal_column, terminal_row);

	if (++terminal_column == VGA_WIDTH) {
		terminal_column = 0;
		if (++terminal_row == VGA_HEIGHT) {
			scrollup();
			terminal_row = VGA_HEIGHT - 1;
		}
	}
}

void terminal_write(const char* data, size_t size)
{
	size_t i, j;

	for (i = 0; i < size; i++) {
		if (data[i] == '\t') {
			for (j = 0; j < (8 - terminal_column % 8); ++j)
				terminal_putchar(' ');
			continue;
		}

		terminal_putchar(data[i]);
	}
}

void terminal_writestring(const char* data) {
	terminal_write(data, strlen(data));
}
