#include <kernel/future.h>
#include <kernel/io/rtc.h>
#include <kernel/monotonic_tick.h>
#include <kernel/panic.h>
#include <stdio.h>

#include "utils.h"

struct FutureList futureList;
bool SHOULD_POLL = false;

Future create_future(uint32_t seconds, IS_READY is_ready, RESUME_FUNC resume_func) {
    Future fut;
    fut.tick = get_tick() + seconds * RTC_FREQ;
    fut.is_ready = is_ready;
    fut.resume_func = resume_func;

    return fut;
}

void init_futures() {
    futureList.lastIdx = -1;
}

enum FutureStatus { PENDING = 0, DONE };

typedef enum FutureStatus FutureStatus;

static FutureStatus poll(Future fut) {
    if (get_tick() >= fut.tick) return DONE;
    return PENDING;
}

void await(Future fut) {
    INTERRUPT_GUARDED({
        if (futureList.lastIdx == FUTURE_COUNT) panic("Future list full!");
        futureList.timeFutures[++futureList.lastIdx] = fut;
    });

    while (poll(fut) == PENDING) {
        SHOULD_POLL = false;

        while (!SHOULD_POLL) {
            asm volatile("hlt");
        }
    }

    // TODO delete it
}

void process_time_futures() {
    // if current tick exists in our set, set flag to true
    SHOULD_POLL = false;
    INTERRUPT_GUARDED({
        if (futureList.lastIdx != -1) {
            if (get_tick() >= futureList.timeFutures[0].tick) SHOULD_POLL = true;
        }
    });
}
