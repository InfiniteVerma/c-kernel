#ifndef CIRCULAR_BUFFER
#define CIRCULAR_BUFFER

#define NUM_LOG_LINES 10
#define LINE_SIZE 100

#define FULL_IDX NUM_LOG_LINES

#define BUFFER_FULL -1
#define LOG_SIZE_OVERFLOW -2

int write_to_buffer(const char* fmt, ...);
void dump_buffer();
int read_from_buffer(char* msg, int size);

#endif
