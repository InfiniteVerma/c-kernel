#include <limits.h>
#include <stdbool.h>
#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include <kernel/panic.h>
#include <utils.h>

char msg[100];
static bool print(const char* data, size_t length) {
	const unsigned char* bytes = (const unsigned char*) data;
	for (size_t i = 0; i < length; i++)
		if (putchar(bytes[i]) == EOF)
			return false;
	return true;
}

static void reverse(int len) {
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

static const char* int_to_hex_char(unsigned long long inp) {
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
    reverse(i);

    return msg;
}

static const char* to_str(int val) {
    bool isNeg = val < 0;
    if(isNeg) val = -1 * val;

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
    if(isNeg) {
        msg[i] = '-';
        i++;
    }
    msg[i] = '\0';
    reverse(i);
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

// Returns num of chars written or -ve if error
int vsnprintf(char* buffer, size_t bufsz, const char* fmt, va_list vlist) {

    // loop through fmt from left. if '%' matched:
    // 1. '%': literal '%'
    // 2. s: character string
    // 3. d: signed int
    int i = 0;

    int bufIdx = 0, fmtIdx = 0;
    while(fmtIdx < bufsz && fmt[fmtIdx] != '\0') {
        if(fmt[fmtIdx] != '%') {
            buffer[bufIdx++] = fmt[fmtIdx++];
        } else {
            fmtIdx++; // consume the '%'
            switch(fmt[fmtIdx]) {
                case '%': buffer[bufIdx++] = '%';
                          break;
                case 's': {
                              const char* s = va_arg(vlist, const char*);
                              int charsToWrite = min((int)strlen(s), bufsz - bufIdx - 1);
                              memcpy(buffer + bufIdx, s,  charsToWrite);
                              bufIdx = bufIdx + charsToWrite;
                              break;
                }
                case 'd': {
                              int i = va_arg(vlist, int);
                              const char* s = to_str(i);
                              int charsToWrite = min((int)strlen(s), bufsz - bufIdx - 1);
                              memcpy(buffer + bufIdx, s,  charsToWrite);
                              bufIdx = bufIdx + charsToWrite;
                              break;
                }
                default: panic("Unsupported vsnprintf\n");
            }
            fmtIdx++;
        }
    }

    buffer[bufIdx] = '\0';
    return bufIdx;
}

#ifdef TEST
void test_printf() {
    memset(msg, 0, sizeof(msg));
}

void test_reverse() {
    char copy[100];
    memcpy(copy, msg, sizeof(msg));

    memset(msg, 0, sizeof(msg));

    char test[4] = "123";
    char test_reverse[4] = "321";
    memcpy(msg, test, sizeof(test));
    reverse(sizeof(test) - 1);

    assert(memcmp(msg, test_reverse, sizeof(test)) == 0, "test_reverse FAILED");

    memcpy(msg, copy, sizeof(msg));
}

void test_to_str() {
    assert(memcmp("123", to_str(123), sizeof("123")) == 0, "test_to_str FAILED");
    assert(memcmp("1", to_str(2), sizeof("1")) != 0, "test_to_str FAILED");
    assert(memcmp("-1", to_str(-1), sizeof("-1")) == 0, "test_to_str FAILED");
    assert(memcmp("-134", to_str(-134), sizeof("-134")) == 0, "test_to_str FAILED");
}

const char* test_vsnprintf_fn(int* outSize, const char* msg, int bufSize, const char* fmt, ...) {
    va_list args;
    va_start(args, fmt);
    *outSize = vsnprintf(msg, bufSize, fmt, args);
    va_end(args);
    return msg;
}

void test_vsnprintf() {
    char out_buf[100] = {0};
    char* out;
    int outSize = 0;

    out = test_vsnprintf_fn(&outSize, out_buf, 100, "test");
    assert(outSize == 4, "test_vsnprintf_1() ret FAILED");
    assert(memcmp(out, "test", sizeof("test")) == 0, "test_vsnprintf_1() FAILED");

    memset(out_buf, 100, sizeof(out_buf));

    out = test_vsnprintf_fn(&outSize, out_buf, 100, "%d", 1);
    assert(outSize == 1, "test_vsnprintf_2() ret FAILED");
    assert(memcmp(out, "1", sizeof("1")) == 0, "test_vsnprintf_2() FAILED");

    memset(out_buf, 100, sizeof(out_buf));

    out = test_vsnprintf_fn(&outSize, out_buf, 100, "%d%d", 1, 23);
    assert(outSize == 3, "test_vsnprintf_3() ret FAILED");
    assert(memcmp(out, "123", sizeof("123")) == 0, "test_vsnprintf_3() FAILED");

    memset(out_buf, 100, sizeof(out_buf));

    out = test_vsnprintf_fn(&outSize, out_buf, 100, "test%d%d", 1, 23);
    assert(outSize == 7, "test_vsnprintf_4() ret FAILED");
    assert(memcmp(out, "test123", sizeof("test123")) == 0, "test_vsnprintf_4() FAILED");

    memset(out_buf, 100, sizeof(out_buf));

    out = test_vsnprintf_fn(&outSize, out_buf, 100, "test%d%d%s%d", 1, 23, "anant", 2000);
    assert(outSize == 16, "test_vsnprintf_5() ret FAILED");
    assert(memcmp(out, "test123anant2000", sizeof("test123anant2000")) == 0, "test_vsnprintf_5() FAILED");

    memset(out_buf, 100, sizeof(out_buf));

    out = test_vsnprintf_fn(&outSize, out_buf, 100, "anant %d <%s> %d END", 1, "str_here", -1);
    assert(memcmp(out, "anant 1 <str_here> -1 END", sizeof("anant 1 <str_here> -1 END")) == 0, "test_vsnprintf_6() FAILED");
}

void run_stdio_tests() {
    test_printf();
    test_reverse();
    test_to_str();
    test_vsnprintf();
    printf("Stdio: [OK]\n");
}
#endif
