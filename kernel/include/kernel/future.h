#ifndef __FUTURE__
#define __FUTURE__

#include <stdbool.h>
#include <stdint.h>

#define FUTURE_COUNT 10

typedef bool (*IS_READY)(void*);
typedef void (*RESUME_FUNC)(void*);

// TODO union for state?

struct Future {
    uint32_t tick;
    IS_READY is_ready;
    RESUME_FUNC resume_func;
};

typedef struct Future Future;

struct FutureList {
    struct Future timeFutures[FUTURE_COUNT];  // TODO assume that this is sorted
    int lastIdx;
};

void init_futures();
void await(Future);
void process_time_futures();
Future create_future(uint32_t, IS_READY, RESUME_FUNC);

#endif
