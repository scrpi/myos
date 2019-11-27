#include <limits.h>
#include <stdbool.h>
#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include <stdint.h>

static bool print(const char* data, size_t length)
{
	const unsigned char* bytes = (const unsigned char*) data;
	size_t i;

	for (i = 0; i < length; i++) {
		if (putchar(bytes[i]) == EOF)
			return false;
	}

	return true;
}

static int print_base(uint64_t val, size_t val_len, int base, size_t maxrem)
{
	char buf[20];
	char *s = buf;
	size_t len = 0;
	unsigned i;

	do {
		int digit = val % base;
		if (base == 16 && digit > 9)
			*s++ = digit - 10 + 'a';
		else
			*s++ = digit + '0';
		val /= base;
		len++;
		if (base == 16 && len == val_len * 2)
			break;
	} while (val);

#if 0
	/* Left-pad hexadecimal */
	while (base == 16 && len != val_len * 2) {
		*s++ = '0';
		len++;
	}
#endif

	if (maxrem < len)
		return -1;

	for (i = 0; i < len / 2; ++i) {
		char tmp = buf[i];
		buf[i] = buf[len - i - 1];
		buf[len - i - 1] = tmp;
	}

	print(buf, len);

	return len;
}

int printf(const char* format, ...) {
	va_list parameters;
	int size_spec = 0;
	int written = 0;

	va_start(parameters, format);

	while (*format != '\0') {
		size_t maxrem = INT_MAX - written;

		if (format[0] != '%' || format[1] == '%') {
			size_spec = 0;
			if (format[0] == '%')
				format++;
			size_t amount = 1;
			while (format[amount] && format[amount] != '%')
				amount++;
			if (maxrem < amount) {
				return -1;
			}
			if (!print(format, amount))
				return -1;
			format += amount;
			written += amount;
			continue;
		}

		const char* format_begun_at = format++;

		while (*format == 'l' && size_spec < 8) {
			size_spec += 4;
			format++;
		}

		if (*format == 'c') {
			format++;
			char c = (char) va_arg(parameters, int /* char promotes to int */);
			if (!maxrem) {
				return -1;
			}
			if (!print(&c, sizeof(c)))
				return -1;
			written++;
		} else if (*format == 'd' && size_spec < 8) {
			int32_t d = (int)va_arg(parameters, int32_t);
			int len;

			if (d < 0) {
				if (!maxrem)
					return -1;
				print("-", 1);
				d = -d;
				written++;
			}

			if ((len = print_base(d, sizeof(d), 10, maxrem)) < 0)
				return -1;

			written += len;
			format++;
		} else if (*format == 'd') {
			int64_t d = (int)va_arg(parameters, int64_t);
			int len;

			if (d < 0) {
				if (!maxrem)
					return -1;
				print("-", 1);
				d = -d;
				written++;
			}

			if ((len = print_base(d, sizeof(d), 10, maxrem)) < 0)
				return -1;

			written += len;
			format++;
		} else if (*format == 'x' && size_spec < 8) {
			int32_t d = (int)va_arg(parameters, int32_t);
			int len;

			if ((len = print_base(d, sizeof(d), 16, maxrem)) < 0)
				return -1;

			written += len;
			format++;
		} else if (*format == 'x') {
			int64_t d = (int)va_arg(parameters, int64_t);
			int len;

			if ((len = print_base(d, sizeof(d), 16, maxrem)) < 0)
				return -1;

			written += len;
			format++;
		} else if (*format == 'p') {
			uint32_t p = va_arg(parameters, uint32_t);
			int len;

			print("0x", 2);
			written += 2;

			if ((len = print_base(p, sizeof(p), 16, maxrem)) < 0)
				return -1;

			written += len;
			format++;
		} else if (*format == 's') {
			format++;
			const char* str = va_arg(parameters, const char*);
			size_t len = strlen(str);
			if (maxrem < len) {
				return -1;
			}
			if (!print(str, len))
				return -1;
			written += len;
		} else {
			format = format_begun_at;
			size_t len = strlen(format);
			if (maxrem < len) {
				return -1;
			}
			if (!print(format, len))
				return -1;
			written += len;
			format += len;
		}
	}

	va_end(parameters);
	return written;
}
