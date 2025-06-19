#include <kernel/io/rtc.h>
#include <kernel/circular_buffer.h>
#include <kernel/panic.h>
#include <string.h>
#include <stdio.h>

char BUFFER[NUM_LOG_LINES][LINE_SIZE];
int head = 0;
int tail = 0;

// TODO: Currently we will write till buffer completes and then clear it entirely 
// so it's not being used as a circular buffer currently
//
// When implementing manual flush, background thread or flush-on-panic, need to handle this
int write_to_buffer(const char* fmt, ...) {
    asm volatile("cli");
    //int size = strlen(msg);
    if(tail == FULL_IDX - 1) {
        // empty buffer first
        // then write
        dump_buffer();
        assert(tail == 0, "tail should be zero after dump_buffer()");
    }

    char time_buf[100] = {0};
    int time_size = get_timestamp(time_buf);

    char buf[100] = {0};
    va_list args;
    va_start(args, fmt);
    int size = vsnprintf(buf, 100, fmt, args);

    if(size > LINE_SIZE)
        return LOG_SIZE_OVERFLOW;

    memcpy(BUFFER[tail], time_buf, time_size);
    memcpy(BUFFER[tail++] + time_size, buf, size);

    va_end(args);
    asm volatile("sti");
    return 0;
}

void dump_buffer() {
    for(int i=0;i<tail;i++) {
        printf("%s\n", BUFFER[i]);
    }
    tail = 0;
}

int read_from_buffer(char* msg, int size) {
    printf("read_from_buffer BEGIN\n");
    printf("read_from_buffer END\n");
    return 0;
}
