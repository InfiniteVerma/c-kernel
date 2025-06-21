#ifndef CIRCULAR_BUFFER
#define CIRCULAR_BUFFER

#define NUM_LOG_LINES 50
#define LINE_SIZE 900

#define FULL_IDX NUM_LOG_LINES

#define BUFFER_FULL -1
#define LOG_SIZE_OVERFLOW -2

int write_to_buffer(const char* fmt, const char* level, const char* file_name, int line_number,
                    ...);
int write_to_buffer_colored(const char* fmt, const char* color, const char* level,
                            const char* file_name, int line_number, ...);
void dump_buffer();
int read_from_buffer(char* msg, int size);

#endif
