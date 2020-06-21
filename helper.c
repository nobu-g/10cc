#include <stdlib.h>

int alloc4(int **pp, int a, int b, int c, int d) {
    *pp = malloc(sizeof(int) * 4);
    *(*pp + 0) = a;
    *(*pp + 1) = b;
    *(*pp + 2) = c;
    *(*pp + 3) = d;
    return 0;
}
