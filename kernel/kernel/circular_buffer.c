#include <kernel/circular_buffer.h>
#include <kernel/io/rtc.h>
#include <kernel/panic.h>
#include <stdio.h>
#include <string.h>
#include <utils.h>

char BUFFER[NUM_LOG_LINES][LINE_SIZE];
int head = 0;
int tail = 0;

static int build_prefix(char* out, const char* fmt, ...) {
    va_list args;
    va_start(args, fmt);
    int size = vsnprintf(out, 100, fmt, args);
    va_end(args);
    return size;
}

int write_to_buffer_colored(const char* fmt, const char* color, const char* level,
                            const char* file_name, int line_number, ...) {
    asm volatile("cli");
    if (tail == FULL_IDX - 1) {
        // empty buffer first
        // then write
        dump_buffer();
        assert(tail == 0, "tail should be zero after dump_buffer()");
    }

    char buf[100] = {0};
    va_list args;
    va_start(args, line_number);
    int size = vsnprintf(buf, 100, fmt, args);

    char prefix[100] = {0};
    int prefix_size = build_prefix(prefix, "%s [%s] %s:%d ", color, level, file_name, line_number);

    int totalSize = size + prefix_size;

    if (totalSize > LINE_SIZE) {
        panic("LOG_SIZE_OVERFLOW");
        return LOG_SIZE_OVERFLOW;  // TODO handle error gracefully
    }

    memcpy(BUFFER[tail], prefix, prefix_size);
    memcpy(BUFFER[tail] + prefix_size, buf, size);
    memcpy(BUFFER[tail++] + prefix_size + size, RESET, strlen(RESET));

    va_end(args);
    asm volatile("sti");
    return 0;
}

// TODO: Currently we will write till buffer completes and then clear it entirely
// so it's not being used as a circular buffer currently
//
// When implementing manual flush, background thread or flush-on-panic, need to handle this
int write_to_buffer(const char* fmt, const char* level, const char* file_name, int line_number,
                    ...) {
    asm volatile("cli");
    if (tail == FULL_IDX - 1) {
        // empty buffer first
        // then write
        dump_buffer();
        assert(tail == 0, "tail should be zero after dump_buffer()");
    }

    char buf[100] = {0};
    va_list args;
    va_start(args, line_number);
    int size = vsnprintf(buf, 100, fmt, args);

    char prefix[100] = {0};
    int prefix_size = build_prefix(prefix, "[%s] %s:%d ", level, file_name, line_number);

    int totalSize = size + prefix_size;

    if (totalSize > LINE_SIZE) {
        panic("LOG_SIZE_OVERFLOW");
        return LOG_SIZE_OVERFLOW;  // TODO handle error gracefully
    }

    memcpy(BUFFER[tail], prefix, prefix_size);
    memcpy(BUFFER[tail++] + prefix_size, buf, size);

    va_end(args);
    asm volatile("sti");
    return 0;
}

void dump_buffer() {
    for (int i = 0; i < tail; i++) {
        printf("%s\n", BUFFER[i]);
    }
    tail = 0;
}

int read_from_buffer(char* msg, int size) {
    LOG("read_from_buffer BEGIN");
    LOG("read_from_buffer END");
    return 0;
}
