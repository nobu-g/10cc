#include "10cc.h"

int main(int argc, char **argv) {
    if (argc != 2) {
        fprintf(stderr, "%s: invalid number of arguments\n", argv[0]);
        return 1;
    }

    user_input = argv[1];
#if DEBUG <= 2
    fprintf(stderr, "received user input\n");
#endif
#if DEBUG <= 1
    fprintf(stderr, "content: %s\n", user_input);
#endif

    tokenize();
#if DEBUG <= 2
    fprintf(stderr, "tokenized user input\n");
#endif
#if DEBUG <= 1
    fprintf(stderr, "tokens:");
    Token *tok = token;
    while (tok->kind != TK_EOF) {
        if (tok->kind != TK_NUM) {
            fprintf(stderr, " %s", tok->str);
        } else {
            fprintf(stderr, " %d", tok->val);
        }
        tok = tok->next;
    }
    fprintf(stderr, "\n");
#endif

    Program *prog = parse();
#if DEBUG <= 2
    fprintf(stderr, "parsed tokens syntactically\n");
#endif

    sema(prog);
#if DEBUG <= 2
    fprintf(stderr, "parsed tokens semantically\n");
#endif
#if DEBUG <= 1
    draw_ast(prog);
#endif

    gen_x86_64(prog);
    return 0;
}
