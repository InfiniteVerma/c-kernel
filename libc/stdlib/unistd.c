#include <kernel/io/rtc.h>
#include <unistd.h>

void sleep(uint32_t time_s) {
    uint32_t start = get_tick();
    uint32_t end = start + time_s * 256;

    while (get_tick() < end) asm volatile("hlt");
}
