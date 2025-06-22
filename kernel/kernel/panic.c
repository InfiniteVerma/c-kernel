#include <kernel/panic.h>
#include <stdio.h>

#include "kernel/circular_buffer.h"
#include "kernel/io/uart.h"

void panic(const char* msg) {
    asm volatile("cli");
    dump_buffer();
    printf("\npanic called with error: %s", msg);

    while (1) {
        asm volatile("hlt");
    }
}

void assert(int cond, const char* msg) {
    if (cond) {
    } else {
        panic(msg);
    }
}
