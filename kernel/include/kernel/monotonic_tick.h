#ifndef __MONOTONIC_TICK__
#define __MONOTONIC_TICK__

#include <stdint.h>

typedef struct {
    uint32_t tick;
} MonotonicTick;

void process_tick();
uint32_t get_tick();

#endif
