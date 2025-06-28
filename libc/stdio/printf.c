#include <kernel/panic.h>
#include <limits.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <utils.h>

char msg[100];
static bool print(const char* data, size_t length) {
    const unsigned char* bytes = (const unsigned char*)data;
    for (size_t i = 0; i < length; i++)
        if (putchar(bytes[i]) == EOF) return false;
    return true;
}

static void int_to_hex(char* buf, uint64_t val, int num_digits) {
    const char* digits = "0123456789abcdef";
    for (int i = num_digits - 1; i >= 0; i--) {
        buf[i] = digits[val & 0xF];
        val >>= 4;
    }
    buf[num_digits] = '\0';
}

int printf(const char* restrict format, ...) {
    char buf[1024];
    va_list args;
    va_start(args, format);
    int written = vsnprintf(buf, sizeof(buf), format, args);
    va_end(args);
    print(buf, written);
    return written;
}

void to_str_unsigned(char* buf, uint64_t value) {
    to_str(buf, value);
}

int vsnprintf(char* buffer, size_t bufsz, const char* format, va_list vlist) {
    size_t written = 0;
    char* out = buffer;

    while (*format) {
        if (*format != '%') {
            if (written + 1 < bufsz) {
                *out++ = *format;
            }
            written++;
            format++;
            continue;
        }

        const char* start_fmt = format++;  // skip '%'

        // Handle %%
        if (*format == '%') {
            if (written + 1 < bufsz) {
                *out++ = '%';
            }
            written++;
            format++;
            continue;
        }

        // Handle format specifier
        char temp[32] = {0};
        size_t len = 0;

        if (*format == 'c') {
            char c = (char)va_arg(vlist, int);
            if (written + 1 < bufsz) {
                *out++ = c;
            }
            written++;
            format++;
            continue;

        } else if (*format == 's') {
            const char* s = va_arg(vlist, const char*);
            size_t slen = strlen(s);
            for (size_t i = 0; i < slen; i++) {
                if (written + 1 < bufsz) {
                    *out++ = s[i];
                }
                written++;
            }
            format++;
            continue;

        } else if (*format == 'd') {
            int x = va_arg(vlist, int);
            to_str(temp, x);
            len = strlen(temp);
            goto copy_temp;

        } else if (*format == 'u') {
            unsigned int x = va_arg(vlist, unsigned int);
            to_str_unsigned(temp, x);
            len = strlen(temp);
            goto copy_temp;

        } else if (*format == 'x') {
            unsigned int x = va_arg(vlist, unsigned int);
            int_to_hex(temp, x, 8);
            len = strlen(temp);
            goto copy_temp;

        } else if (strncmp(format, "llx", 3) == 0) {
            format += 2;  // we'll increment once more below
            unsigned long long x = va_arg(vlist, unsigned long long);
            int_to_hex(temp, x, 16);
            len = strlen(temp);
            goto copy_temp;

        } else if (strncmp(format, "llu", 3) == 0) {
            format += 2;
            unsigned long long x = va_arg(vlist, unsigned long long);
            to_str_unsigned(temp, x);
            len = strlen(temp);
            goto copy_temp;

        } else if (strncmp(format, "lu", 2) == 0) {
            format += 1;
            unsigned long x = va_arg(vlist, unsigned long);
            to_str_unsigned(temp, x);
            len = strlen(temp);
            goto copy_temp;
        }

        // Unknown format specifier, just print as-is
        while (*start_fmt && *start_fmt != '%') start_fmt++;
        while (start_fmt <= format) {
            if (written + 1 < bufsz) *out++ = *start_fmt;
            written++;
            start_fmt++;
        }
        format++;
        continue;

    copy_temp:
        for (size_t i = 0; i < len; i++) {
            if (written + 1 < bufsz) {
                *out++ = temp[i];
            }
            written++;
        }
        format++;
    }

    // Null-terminate if space allows
    if (bufsz > 0) {
        if (written >= bufsz)
            buffer[bufsz - 1] = '\0';
        else
            buffer[written] = '\0';
    }

    return written;
}

#ifdef TEST
void test_printf() {
    memset(msg, 0, sizeof(msg));
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
    assert(memcmp(out, "test123anant2000", sizeof("test123anant2000")) == 0,
           "test_vsnprintf_5() FAILED");

    memset(out_buf, 100, sizeof(out_buf));

    out = test_vsnprintf_fn(&outSize, out_buf, 100, "anant %d <%s> %d END", 1, "str_here", -1);
    assert(memcmp(out, "anant 1 <str_here> -1 END", sizeof("anant 1 <str_here> -1 END")) == 0,
           "test_vsnprintf_6() FAILED");
}

void run_stdio_tests() {
    test_printf();
    test_vsnprintf();
    LOG_GREEN("Stdio: [OK]");
}
#endif
