DEFAULT_HOST!=../default-host.sh
HOST?=DEFAULT_HOST
HOSTARCH!=../target-triplet-to-arch.sh $(HOST)

CFLAGS?=-O2 -g
CPPFLAGS?=
LDFLAGS?=
LIBS?=

DESTDIR?=
PREFIX?=/usr/local
EXEC_PREFIX?=$(PREFIX)
BOOTDIR?=$(EXEC_PREFIX)/boot
INCLUDEDIR?=$(PREFIX)/include

CFLAGS:=$(CFLAGS) -ffreestanding -Wall -Wextra
CPPFLAGS:=$(CPPFLAGS) -D__is_kernel -I. -Ilib
LDFLAGS:=$(LDFLAGS)
LIBS:=$(LIBS) -nostdlib -lgcc

LIB_OBJS=\
	lib/stdio/printf.o \
	lib/stdio/putchar.o \
	lib/stdio/puts.o \
	lib/stdlib/abort.o \
	lib/stdlib/malloc.o \
	lib/string/memcmp.o \
	lib/string/memcpy.o \
	lib/string/memmove.o \
	lib/string/memset.o \
	lib/string/strlen.o \
	lib/string/strcmp.o \

KERNEL_OBJS=\
	boot/boot.o \
	kernel.o \
	mm/pmm.o \
	mm/vmm.o \
	mm/mmu.o \
	alloc.o \
	gdt.o \
	trap.o \
	trap_asm.o \
	vectors.o \
	timer.o \
	drivers/uart.o \
	drivers/tty.o

#	$(ARCHDIR)/crti.o \
#	$(ARCHDIR)/crtbegin.o \
#	...
#	$(ARCHDIR)/crtend.o \
#	$(ARCHDIR)/crtn.o
#
OBJS=\
	$(LIB_OBJS) \
	$(KERNEL_OBJS)

LINK_LIST=\
	$(LDFLAGS) \
	$(KERNEL_OBJS) \
	$(LIB_OBJS) \
	$(LIBS)

.PHONY: all clean install install-kernel
.SUFFIXES: .o .c .S

all: kernel

kernel: $(OBJS) linker.ld
	$(CC) -T linker.ld -o $@ $(CFLAGS) $(LINK_LIST)
	grub-file --is-x86-multiboot kernel

$(ARCHDIR)/crtbegin.o $(ARCHDIR)/crtend.o:
	OBJ=`$(CC) $(CFLAGS) $(LDFLAGS) -print-file-name=$(@F)` && cp "$$OBJ" $@

.c.o:
	$(CC) -MD -c $< -o $@ -std=c99 $(CFLAGS) $(CPPFLAGS)

.S.o:
	$(CC) -MD -c $< -o $@ $(CFLAGS) $(CPPFLAGS)

clean:
	rm -f kernel
	rm -f $(OBJS) *.o */*.o */*/*.o
	rm -f $(OBJS:.o=.d) *.d */*.d */*/*.d

install: install-kernel

install-kernel: kernel
	mkdir -p $(DESTDIR)$(BOOTDIR)
	cp kernel $(DESTDIR)$(BOOTDIR)

-include $(OBJS:.o=.d)
