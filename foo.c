#include <stdio.h>

void foo(int x, int y, int z, int a, int b) {
    printf("OK %d + %d + %d + %d + %d = %d\n", x, y, z, a, b, x + y + z + a + b);
}
void var(int x, int y, int z, int a) {
    printf("OK %d + %d + %d + %d = %d\n", x, y, z, a, x + y + z + a);
}
