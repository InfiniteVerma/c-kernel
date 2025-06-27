#ifndef __FUTURE__
#define __FUTURE__

#include <stdbool.h>
#include <stdint.h>

#define FUTURE_COUNT 10

typedef bool (*IS_READY)(void*);
typedef void (*RESUME_FUNC)(void*);

enum FutureStatus { PENDING = 0, DONE };

enum FutureType { SleepFuture = 0, IOFuture };

typedef enum FutureStatus FutureStatus;
typedef enum FutureType FutureType;

struct SleepContext {
    uint32_t target_tick;
};

struct Future {
    FutureType type;
    void* context;
    IS_READY is_ready;
};

struct FutureList {
    struct Future timeFutures[FUTURE_COUNT];  // TODO assume that this is sorted
    int lastIdx;
};

typedef struct Future Future;
typedef struct SleepContext SleepContext;

void init_futures();
void await(Future);
void process_time_futures();
Future create_future(uint32_t, IS_READY, RESUME_FUNC);
void delete_future(Future);
void wakeup_executor();

#endif
