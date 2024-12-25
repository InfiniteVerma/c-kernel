#include <limits.h>
#include <stdbool.h>
#include <stdarg.h>
#include <stdio.h>
#include <string.h>

char msg[100];
static bool print(const char* data, size_t length) {
	const unsigned char* bytes = (const unsigned char*) data;
	for (size_t i = 0; i < length; i++)
		if (putchar(bytes[i]) == EOF)
			return false;
	return true;
}

void reverse(const char* ptr, int len) {
    int start = 0;
    int end = len - 1; 
    while(start < end) {
        char c = msg[start];
        msg[start] = msg[end];
        msg[end] = c;
        start++;
        end--;
    }
}

static const char* int_to_hex_char(int inp) {

    int i = 0;
    memset(msg, 0, sizeof(msg));
    if(inp == 0) {
        msg[0] = '0';
        msg[1] = '\0';
        return msg;
    }
    while(inp) {
        if (inp % 16 < 10) {
            msg[i] = (inp % 16) + '0';  // For digits 0-9
        } else {
            msg[i] = (inp % 16) - 10 + 'a';  // For characters a-f
        }
        i++;
        inp /= 16;
    }
    msg[i] = '\0';
    reverse(msg, i);

    return msg;
}

// TODO do negative
static const char* to_str(int val) {

    int tmp = val;
    int i = 0;
    memset(msg, 0, sizeof(msg));
    if(val == 0) {
        msg[0] = '0';
        msg[1] = '\0';
        return msg;
    }
    while(tmp) {
        msg[i] = tmp % 10 + '0';
        tmp /= 10;
        i++;
    }
    msg[i] = '\0';
    reverse(msg, i);
    return msg;
}

int printf(const char* restrict format, ...) {
	va_list parameters;
	va_start(parameters, format);

	int written = 0;

	while (*format != '\0') {
		size_t maxrem = INT_MAX - written;

		if (format[0] != '%' || format[1] == '%') {
			if (format[0] == '%')
				format++;
			size_t amount = 1;
			while (format[amount] && format[amount] != '%')
				amount++;
			if (maxrem < amount) {
				// TODO: Set errno to EOVERFLOW.
				return -1;
			}
			if (!print(format, amount))
				return -1;
			format += amount;
			written += amount;
			continue;
		}

		const char* format_begun_at = format++;

		if (*format == 'c') {
			format++;
			char c = (char) va_arg(parameters, int /* char promotes to int */);
			if (!maxrem) {
				// TODO: Set errno to EOVERFLOW.
				return -1;
			}
			if (!print(&c, sizeof(c)))
				return -1;
			written++;
		} else if (*format == 's') {
			format++;
			const char* str = va_arg(parameters, const char*);
			size_t len = strlen(str);
			if (maxrem < len) {
				// TODO: Set errno to EOVERFLOW.
				return -1;
			}
			if (!print(str, len))
				return -1;
			written += len;
		} else if (*format == 'd') {
            format++;
            int x = va_arg(parameters, int);
            const char* msg = to_str(x);
			size_t len = strlen(msg);
			if (maxrem < len) {
				// TODO: Set errno to EOVERFLOW.
				return -1;
			}
			if (!print(msg, len))
				return -1;
			written += len;
        } else if (*format == 'u') {
            format++;
            unsigned int x = va_arg(parameters, unsigned int);
            const char* msg = to_str(x);
			size_t len = strlen(msg);
			if (maxrem < len) {
				// TODO: Set errno to EOVERFLOW.
				return -1;
			}
			if (!print(msg, len))
				return -1;
			written += len;
        } else if (*format == 'x') { // hex
            format++;
            unsigned long long x = va_arg(parameters, unsigned long long); // TODO?
            const char* msg = int_to_hex_char(x);
			size_t len = strlen(msg);
			if (maxrem < len) {
				// TODO: Set errno to EOVERFLOW.
				return -1;
			}
			if (!print(msg, len))
				return -1;
			written += len;
        } else if(memcmp(format, "llu", 3) == 0) {
            format+=3;
            unsigned long long x = va_arg(parameters, unsigned long long);
            const char* msg = to_str(x);
			size_t len = strlen(msg);
			if (maxrem < len) {
				// TODO: Set errno to EOVERFLOW.
				return -1;
			}
			if (!print(msg, len))
				return -1;
			written += len;
        } else if(memcmp(format, "lu", 2) == 0) {
            format+=2;
            unsigned long x = va_arg(parameters, unsigned long);
            const char* msg = to_str(x);
			size_t len = strlen(msg);
			if (maxrem < len) {
				// TODO: Set errno to EOVERFLOW.
				return -1;
			}
			if (!print(msg, len))
				return -1;
			written += len;
        } else {
			format = format_begun_at;
			size_t len = strlen(format);
			if (maxrem < len) {
				// TODO: Set errno to EOVERFLOW.
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
