#ifndef __UART__
#define __UART__

#include <stdint.h>
int init_serial();

void outb(uint16_t, uint8_t);
uint8_t inb(uint16_t);
void serial_write(const char* msg, int len);
void serial_putchar(const char c);

#endif
