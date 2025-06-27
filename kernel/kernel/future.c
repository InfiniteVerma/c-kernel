#include <kernel/allocator.h>
#include <kernel/future.h>
#include <kernel/io/rtc.h>
#include <kernel/monotonic_tick.h>
#include <kernel/panic.h>
#include <stdio.h>

#include "utils.h"

struct FutureList futureList;
bool SHOULD_POLL = false;

SleepContext* alloc_sleep_context() {
    SleepContext* ctx = (SleepContext*)malloc(sizeof(SleepContext));
    return ctx;
}

Future create_future(uint32_t seconds, IS_READY is_ready, RESUME_FUNC resume_func) {
    SleepContext* ctx = alloc_sleep_context();
    ctx->target_tick = get_tick() + seconds * RTC_FREQ;

    Future fut = {.type = SleepFuture, .context = ctx, .is_ready = is_ready};

    return fut;
}

void init_futures() {
    futureList.lastIdx = -1;
}

static FutureStatus poll(Future fut) {
    if (fut.is_ready(fut.context)) return DONE;
    return PENDING;
}

void await(Future fut) {
    if (fut.type == SleepFuture) {
        INTERRUPT_GUARDED({
            if (futureList.lastIdx == FUTURE_COUNT) panic("Future list full!");
            futureList.timeFutures[++futureList.lastIdx] = fut;
        });
    }

    while (poll(fut) == PENDING) {
        SHOULD_POLL = false;

        while (!SHOULD_POLL) {
            asm volatile("hlt");
        }
    }

    delete_future(fut);
}

void delete_future(Future fut) {
    switch (fut.type) {
        case SleepFuture: {
            free(fut.context);
            // TODO remove from futureList
            break;
        }
        case IOFuture: {
            break;
        }
    }
}

void wakeup_executor() {
    SHOULD_POLL = true;
}

void process_time_futures() {
    // if current tick exists in our set, set flag to true
    SHOULD_POLL = false;
    INTERRUPT_GUARDED({
        if (futureList.lastIdx != -1) {
            SleepContext* ctx = (SleepContext*)futureList.timeFutures[0].context;
            if (get_tick() >= ctx->target_tick) SHOULD_POLL = true;
        }
    });
}
