#include <stdio.h>
#include <stdlib.h>

void print_header() {
    printf(".intel_syntax noprefix\n");
    printf(".global main\n");
}

int main(int argc, char **argv) {
    if(argc != 2) {
        fprintf(stderr, "%s: invalid number of arguments\n", argv[0]);
        return 1;
    }

    char *p = argv[1];

    print_header();
    printf("main:\n");

    printf("  mov rax, %ld\n", strtol(p, &p, 10));
    while(*p) {
        if(*p == '+') {
            p++;
            printf("  add rax, %ld\n", strtol(p, &p, 10));
            continue;
        }
        if(*p == '-') {
            p++;
            printf("  sub rax, %ld\n", strtol(p, &p, 10));
            continue;
        }
        fprintf(stderr, "Invalid character: '%c'\n", *p);
        return 1;
    }

    printf("  ret\n");
    return 0;
}
