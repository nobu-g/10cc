#include "10cc.h"

int main(int argc, char **argv) {
    if (argc != 2) {
        fprintf(stderr, "%s: invalid number of arguments\n", argv[0]);
        return 1;
    }

    File *file = read_file(argv[1]);

    token = tokenize(file);
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
    token = preprocess(token);

    Program *prog = parse();
#if DEBUG <= 2
    fprintf(stderr, "parsed tokens syntactically\n");
#endif
#if DEBUG <= 1
    draw_ast(prog);
#endif

    add_type(prog);
#if DEBUG <= 2
    fprintf(stderr, "parsed tokens semantically\n");
#endif
#if DEBUG <= 1
    draw_ast(prog);
#endif

    gen_x86_64(prog);
    return 0;
}
