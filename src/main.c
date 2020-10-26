#include "10cc.h"

char *filename;
char *user_input;

char* read_file(char* path) {
    // open file
    FILE* fp = fopen(path, "r");
    if (!fp) error("cannot open %s: %s", path, strerror(errno));

    // seek toward end, get file size, and seek again toward start
    if (fseek(fp, 0, SEEK_END) == -1) {
        error("%s: fseek: %s", path, strerror(errno));
    }
    size_t size = ftell(fp);
    if (fseek(fp, 0, SEEK_SET) == -1) {
        error("%s: fseek: %s", path, strerror(errno));
    }

    // move file content to buf
    char* buf = calloc(1, size + 2);  // 2: \n\0
    fread(buf, size, 1, fp);

    // end file with \n and append \0
    if (size == 0 || buf[size - 1] != '\n') {
        buf[size++] = '\n';
    }
    buf[size] = '\0';
    fclose(fp);
    return buf;
}

int main(int argc, char **argv) {
    if (argc != 2) {
        fprintf(stderr, "%s: invalid number of arguments\n", argv[0]);
        return 1;
    }

    filename = argv[1];
    user_input = read_file(filename);
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
