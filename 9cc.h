#include <ctype.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef enum {
    TK_RESERVED,  // 記号
    TK_NUM,       // 整数トークン
    TK_EOF,       // 入力の終わりを表すトークン
} TokenKind;

typedef enum {
    ND_ADD,  // indicates +
    ND_SUB,  // indicates -
    ND_MUL,  // indicates *
    ND_DIV,  // indicates /
    ND_EQ,   // indicates ==
    ND_NE,   // indicates ==
    ND_LE,   // indicates <=
    ND_LT,   // indicates <
    ND_NUM,  // indicates a number
} NodeKind;

typedef struct Token Token;

struct Token {
    TokenKind kind;
    Token *next;
    int val;
    char *str;
    int len;
};

typedef struct Node Node;

struct Node {
    NodeKind kind;
    Node *lhs;
    Node *rhs;
    int val;
};

extern char *user_input;
extern Token *token;

Token *tokenize(char *p);
Node *expr();
Node *equality();
Node *relational();
Node *add();
Node *mul();
Node *unary();
Node *primary();

void gen(Node *node);
