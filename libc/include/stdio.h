#ifndef _STDIO_H
#define _STDIO_H 1

#include <stdarg.h>
#include <stddef.h>
#include <sys/cdefs.h>

#define EOF (-1)

#ifdef __cplusplus
extern "C" {
#endif

int printf(const char* __restrict, ...);
int putchar(int);
int puts(const char*);
int vsnprintf(char* buffer, size_t bufsz, const char* format, va_list vlist);
void run_stdio_tests();

#ifdef __cplusplus
}
#endif

#endif
