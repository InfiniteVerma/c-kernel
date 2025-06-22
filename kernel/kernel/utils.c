#include <stdbool.h>
#include <string.h>
#include <utils.h>
#ifdef TEST
#include <kernel/panic.h>
#include <stdio.h>
#endif

void reverse(char msg[100], int len) {
    int start = 0;
    int end = len - 1;
    while (start < end) {
        char c = msg[start];
        msg[start] = msg[end];
        msg[end] = c;
        start++;
        end--;
    }
}

const char* to_str(char msg[100], int val) {
    bool isNeg = val < 0;
    if (isNeg) val = -1 * val;

    int tmp = val;
    int i = 0;
    memset(msg, 0, sizeof(msg));
    if (val == 0) {
        msg[0] = '0';
        msg[1] = '\0';
        return msg;
    }
    while (tmp) {
        msg[i] = tmp % 10 + '0';
        tmp /= 10;
        i++;
    }
    if (isNeg) {
        msg[i] = '-';
        i++;
    }
    msg[i] = '\0';
    reverse(msg, i);
    return msg;
}

const char* int_to_hex_char(char msg[100], unsigned long long inp) {
    int i = 0;
    memset(msg, 0, sizeof(msg));
    if (inp == 0) {
        msg[0] = '0';
        msg[1] = '\0';
        return msg;
    }
    while (inp) {
        if (inp % 16 < 10) {
            msg[i] = (inp % 16) + '0';  // For digits 0-9
        } else {
            msg[i] = (inp % 16) - 10 + 'a';  // For characters a-f
        }
        i++;
        inp /= 16;
    }
    msg[i] = '\0';
    reverse(msg, i);

    return msg;
}

void outb(uint16_t port, uint8_t value) {
    asm volatile("outb %0, %1"
                 :  // No output operands
                 : "a"(value), "Nd"(port));
}

uint8_t inb(uint16_t port) {
    uint8_t value;
    asm volatile("inb %1, %0" : "=a"(value) : "Nd"(port));
    return value;
}

#ifdef TEST
char test_arr[100];
void test_reverse() {
    char copy[100];
    memcpy(copy, test_arr, sizeof(test_arr));

    memset(test_arr, 0, sizeof(test_arr));

    char test[4] = "123";
    char test_reverse[4] = "321";
    memcpy(test_arr, test, sizeof(test));
    reverse(test_arr, sizeof(test) - 1);

    assert(memcmp(test_arr, test_reverse, sizeof(test)) == 0, "test_reverse FAILED");

    memcpy(test_arr, copy, sizeof(test_arr));
}

void test_to_str() {
    assert(memcmp("123", to_str(test_arr, 123), sizeof("123")) == 0, "test_to_str FAILED");
    assert(memcmp("1", to_str(test_arr, 2), sizeof("1")) != 0, "test_to_str FAILED");
    assert(memcmp("-1", to_str(test_arr, -1), sizeof("-1")) == 0, "test_to_str FAILED");
    assert(memcmp("-134", to_str(test_arr, -134), sizeof("-134")) == 0, "test_to_str FAILED");
}

void run_utils_tests() {
    test_reverse();
    test_to_str();
    LOG_GREEN("Utils: [OK]");
}
#endif
