#include <ctype.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct LVar LVar;

void error(char *fmt, ...);
void error_at(char *loc, char *fmt, ...);
bool startswith(char *p, char *q);
bool is_alnum(char c);

typedef enum {
    TK_RESERVED, // 記号
    TK_IDENT,    // 識別子
    TK_NUM,      // 整数トークン
    TK_RETURN,   // return
    TK_EOF,      // 入力の終わりを表すトークン
} TokenKind;

typedef struct Token Token;

struct Token {
    TokenKind kind;
    Token *next;
    int val;
    char *str;
    int len;
};

// ローカル変数の型
struct LVar {
  LVar *next; // 次の変数かNULL
  char *name; // 変数の名前
  int len;    // 名前の長さ
  int offset; // RBPからのオフセット
};

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
    ND_RETURN,  // return
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
extern LVar *locals;
extern Token *token;

void tokenize();

bool consume(char *op);
bool consume_stmt(int kind);
Token *consume_ident();
void expect(char *op);
int expect_number();

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
