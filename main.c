#include "10cc.h"

int main(int argc, char **argv) {
    if(argc != 2) {
        fprintf(stderr, "%s: invalid number of arguments\n", argv[0]);
        return 1;
    }

    // tokenize and parse
    // the parse result is stored in "code"
    user_input = argv[1];
    tokenize();
    program();
    gen_x86();
    return 0;
}
