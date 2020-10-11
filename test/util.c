#include <stdio.h>

int assert(int expected, int actual, char *code) {
    if (expected == actual) {
        printf("\e[32m[PASSED]\e[m %s \e[33m=> \e[36m%d\e[m\n", code, expected);
        return 0;
    } else {
        printf("\e[31m[FAILED]\e[m %s \e[33m=> \e[36m%d\e[m expected, but got \e[36m%d\e[m\n", code, expected, actual);
        return 1;
    }
}
