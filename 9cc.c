#include <ctype.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void print_header() {
    printf(".intel_syntax noprefix\n");
    printf(".global main\n");
}

typedef enum {
    TK_RESERVED,  // 記号
    TK_NUM,       // 整数トークン
    TK_EOF,       // 入力の終わりを表すトークン
} TokenKind;

typedef struct Token Token;

struct Token {
    TokenKind kind;
    Token *next;
    int val;
    char *str;
};

// 現在着目しているトークン
Token *token;
char *user_input;

void error_at(char *loc, char *fmt, ...) {
    va_list ap;
    va_start(ap, fmt);

    int pos = loc - user_input;
    fprintf(stderr, "%s\n", user_input);
    fprintf(stderr, "%*s", pos, "");  // pos個の空白を出力
    fprintf(stderr, "^ ");
    vfprintf(stderr, fmt, ap);
    fprintf(stderr, "\n");
    exit(1);
}

// returns True if the current token is op
bool consume(char op) {
    if (token->kind != TK_RESERVED || token->str[0] != op) {
        return false;
    }
    token = token->next;
    return true;
}

void expect(char op) {
    if (token->kind != TK_RESERVED || token->str[0] != op) {
        error_at(token->str, "'%c'ではありません", op);
    }
    token = token->next;
}

int expect_number() {
    if (token->kind !=TK_NUM) {
        error_at(token->str, "数ではありません");
    }
    int val = token->val;
    token = token->next;
    return val;
}

bool at_eof() {
    return token->kind == TK_EOF;
}

Token *new_token(TokenKind kind, Token *cur, char *str) {
    Token *tok = calloc(1, sizeof(Token));
    tok->kind = kind;
    tok->str = str;
    cur->next = tok;
    return tok;
}

Token *tokenize(char *p) {
    Token head;
    head.next = NULL;
    Token *cur = &head;

    while (*p) {
        if (isspace(*p)) {
            p++;
            continue;
        }

        if (*p == '+' || *p == '-') {
            cur = new_token(TK_RESERVED, cur, p++);
            continue;
        }

        if (isdigit(*p)) {
            cur = new_token(TK_NUM, cur, p);
            cur->val = strtol(p, &p, 10);
            continue;
        }
        error_at(p, "トークナイズできません");
    }

    new_token(TK_EOF, cur, p);
    return head.next;
}

int main(int argc, char **argv) {
    if(argc != 2) {
        fprintf(stderr, "%s: invalid number of arguments\n", argv[0]);
        return 1;
    }

    user_input = argv[1];
    token = tokenize(argv[1]);  // first token of input

    print_header();
    printf("main:\n");

    printf("  mov rax, %d\n", expect_number());

    while(!at_eof()) {
        if (consume('+')) {
            printf("  add rax, %d\n", expect_number());
            continue;
        }

        expect('-');
        printf("  sub rax, %d\n", expect_number());
        continue;
    }

    printf("  ret\n");
    return 0;
}
