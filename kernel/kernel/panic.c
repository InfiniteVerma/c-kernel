#include <kernel/panic.h>
#include <stdio.h>
#include "kernel/io/uart.h"

void panic(const char* msg) {
    printf("\npanic called with error: %s", msg);

    asm volatile ("cli");
    while (1) {
        asm volatile ("hlt");
    }
}

void assert(int cond, const char* msg) {
    if(cond) {
    } else {
        panic(msg);
    }
}
