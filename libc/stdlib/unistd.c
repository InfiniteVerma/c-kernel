#include <kernel/io/rtc.h>
#include <kernel/monotonic_tick.h>
#include <unistd.h>

void sleep(uint32_t time_s) {
    uint32_t start = get_tick();
    uint32_t end = start + time_s * RTC_FREQ;

    while (get_tick() < end) asm volatile("hlt");
}
