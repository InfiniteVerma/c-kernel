#include <string.h>

int strncmp(const char* str1, const char* str2, size_t num) {
    for (int i = 0; i < num; i++) {
        if (str1[i] == '\0' || str2[i] == '\0')
            break;
        else if (str1[i] == str2[i])
            continue;
        else if (str1[i] < str2[i])
            return -1;
        else
            return 1;
    }

    return 0;
}
