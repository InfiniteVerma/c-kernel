#ifndef __UTILS__
#define __UTILS__

#include <kernel/circular_buffer.h>
#include <stdint.h>

#include "utils.h"

#define GREEN "\033[32m"
#define RESET "\033[0m"

#define min(a, b) ((a) < (b) ? (a) : (b))

#define LOG(x, ...) write_to_buffer(x, "INFO", __FILE__, __LINE__, ##__VA_ARGS__)
#define LOG_GREEN(x, ...) \
    write_to_buffer_colored(x, GREEN, "INFO", __FILE__, __LINE__, ##__VA_ARGS__)

const char* to_str(char msg[100], int);
// const char* int_to_hex_char(char msg[100], unsigned long long inp);
void reverse(char msg[100], int len);

void outb(uint16_t port, uint8_t value);
uint8_t inb(uint16_t port);
void outl(uint16_t port, uint32_t value);
uint32_t inl(uint16_t port);

void run_utils_tests();

void disable_interrupts();
void enable_interrupts();

#define INTERRUPT_GUARDED(code) \
    do {                        \
        disable_interrupts();   \
        code;                   \
        enable_interrupts();    \
    } while (0)

#endif
