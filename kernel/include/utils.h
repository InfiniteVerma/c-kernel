#ifndef __UTILS__
#define __UTILS__

#include <kernel/circular_buffer.h>

#define min(a, b) ((a) < (b) ? (a) : (b))

#define LOG(x, ...) write_to_buffer(x, ##__VA_ARGS__)

#endif
