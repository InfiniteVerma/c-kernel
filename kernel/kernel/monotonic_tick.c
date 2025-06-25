#include <kernel/future.h>
#include <kernel/monotonic_tick.h>
#include <utils.h>

MonotonicTick monotonicTick = {0};

void process_tick() {
    monotonicTick.tick++;
    process_time_futures();
}

uint32_t get_tick() {
    uint32_t tick = 0;
    INTERRUPT_GUARDED({ tick = monotonicTick.tick; });
    return tick;
}
