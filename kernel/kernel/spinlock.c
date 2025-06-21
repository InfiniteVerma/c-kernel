#include <kernel/spinlock.h>
#include <kernel/panic.h>
#ifdef TEST
#include <stdio.h>
#include <utils.h>
#endif

// TODO make a utils?
static int set_val_atomic(uint8_t* ptr, int val) {
    int result = -1;
    __asm__ volatile(
            "xchg %0, %1"
        :   "=r"(result), "+m"(*ptr)
        :   "0"(val)
        :   "memory");
    assert(*ptr == val, "*ptr is not 1");
    return result;
}

int spin_lock(spinlock_t* lock) {
    if(!lock)
        panic("spinlock NULL!");

    while(set_val_atomic(&lock->locked, 1) != 0) {};

    assert(lock->locked == 1, "lock->locked is not 1");
    return 0;
}

int spin_unlock(spinlock_t* lock) {
    if(!lock)
        panic("spinlock NULL!");

    if(lock->locked == 0)
        panic("lock not held, trying to unlock. Asserting");

    lock->locked = 0;

    return 0;
}

int spin_trylock(spinlock_t* lock) {
    if(!lock)
        panic("spinlock NULL!");

    if(set_val_atomic(&lock->locked, 1) == 0)
        return 0;
    else
        return BUSY;
}

#ifdef TEST
void test_atomic_set() {
    uint8_t orig = 1;
    uint8_t new = 2;
    int ret = set_val_atomic(&orig, new);
    assert(orig == 2, "test_atomic_set FAILED");
}
void run_spinlock_tests() {
    test_atomic_set();
    LOG_GREEN("Spinlock Tests: [OK]");
}
#endif
