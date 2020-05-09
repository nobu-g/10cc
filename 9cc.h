#include <ctype.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef enum {
    ND_ADD,     // +
    ND_SUB,     // -
    ND_MUL,     // *
    ND_DIV,     // /
    ND_EQ,      // ==
    ND_NE,      // ==
    ND_LE,      // <=
    ND_LT,      // <
    ND_ASSIGN,  // =
    ND_LVAR,    // local variable
    ND_NUM,     // number
} NodeKind;

typedef struct Node Node;

struct Node {
    NodeKind kind;
    Node *lhs;
    Node *rhs;
    int val;     // used only if kind=ND_NUM
    int offset;  // used only if kind=ND_LVAR
};

extern char *user_input;
extern Node *code[];

void tokenize();
void program();
Node *stmt();
Node *expr();
Node *assign();
Node *equality();
Node *relational();
Node *add();
Node *mul();
Node *unary();
Node *primary();

void gen(Node *node);
