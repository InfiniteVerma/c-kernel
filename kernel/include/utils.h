#ifndef __UTILS__
#define __UTILS__

#include <kernel/circular_buffer.h>

#define GREEN "\033[32m"
#define RESET "\033[0m"

#define min(a, b) ((a) < (b) ? (a) : (b))

#define LOG(x, ...) write_to_buffer(x, "INFO", __FILE__, __LINE__, ##__VA_ARGS__)
#define LOG_GREEN(x, ...) \
    write_to_buffer_colored(x, GREEN, "INFO", __FILE__, __LINE__, ##__VA_ARGS__)

const char* to_str(char msg[100], int);
const char* int_to_hex_char(char msg[100], unsigned long long inp);
void reverse(char msg[100], int len);

void run_utils_tests();

#endif
