#include <kernel/panic.h>
#include <stdio.h>

void panic(const char* msg) {
    printf("\npanic called with error: %s", msg);

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
