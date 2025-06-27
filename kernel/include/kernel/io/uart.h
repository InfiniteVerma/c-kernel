#ifndef __UART__
#define __UART__

#include <kernel/future.h>
#include <stdint.h>

int init_serial();

void serial_write(const char* msg, int len);
void serial_putchar(const char c);
void exit_(const uint8_t);

Future create_serial_future();

#endif
