#ifndef __SPINLOCK__
#define __SPINLOCK__

#include <stdint.h>

#define BUSY -1

typedef struct {
    uint8_t locked;
} spinlock_t;

int spin_lock(spinlock_t*);
int spin_unlock(spinlock_t*);
int spin_trylock(spinlock_t*);

void run_spinlock_tests();

#endif
