#include <ctype.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct LVar LVar;
typedef struct Func Func;

void error(char *fmt, ...);
void error_at(char *loc, char *fmt, ...);
bool startswith(char *p, char *q);
bool is_alnum(char c);

typedef struct {
    void **data;
    int capacity;
    int len;
} Vector;

Vector *create_vector();
void push(Vector *vec, void *elem);

typedef struct {
    Vector *keys;
    Vector *vals;
    int len;
} Map;

Map *create_map();
void add_elem_to_map(Map *map, char *key, void *val);
void *get_elem_from_map(Map *map, char *key);

typedef enum {
    TK_RESERVED, // 記号
    TK_IDENT,    // 識別子
    TK_NUM,      // 整数トークン
    TK_RETURN,   // return
    TK_IF,       // if
    TK_ELSE,     // else
    TK_WHILE,    // while
    TK_FOR,      // for
    TK_EOF,      // 入力の終わりを表すトークン
    TK_INT,      // int
    TK_SIZEOF,   // sizeof
} TokenKind;

typedef struct Token Token;

struct Token {
    TokenKind kind;
    Token *next;
    int val;
    char *str;
    char *loc;
};

// ローカル変数の型
struct LVar {
    char *name; // 変数の名前
    int len;    // 名前の長さ
    int offset; // RBPからのオフセット
};

struct Type {
    enum {INT, PTR} ty;
    int size;  // INT: 4, PTR: 8

    struct Type *ptr_to;
};

typedef struct Type Type;

// 関数の型
struct Func {
    char *name;  // 関数の名前
    Map *lvars;  // ローカル変数(args含む) (Map<char *, LVar *>)
    Vector *args;  // 引数 (Vector<Node *>)
    Vector *body;  // 実装 (Vector<Node *>)
    Type *ret_type;  // 戻り値の型
};

typedef enum {
    ND_ADD,        // +
    ND_SUB,        // -
    ND_MUL,        // *
    ND_DIV,        // /
    ND_EQ,         // ==
    ND_NE,         // ==
    ND_LE,         // <=
    ND_LT,         // <
    ND_ASSIGN,     // =
    ND_RETURN,     // return
    ND_IF,         // if
    ND_ELSE,       // else
    ND_WHILE,      // while
    ND_FOR,        // for
    ND_BLOCK,      // block
    ND_FUNC_CALL,  // function call
    ND_FUNC_DEF,   // function definition
    ND_LVAR,       // local variable
    ND_NUM,        // number
    ND_ADDR,       // unary &
    ND_DEREF,      // unary *
} NodeKind;

typedef struct Node Node;

struct Node {
    NodeKind kind;

    // used for binary operators
    Node *lhs;
    Node *rhs;

    // used for 'if' statement
    Node *cond;
    Node *then;
    Node *els;

    // used for 'for' statement
    Node *init;
    Node *upd;

    // used for block
    Vector *stmts;  // Vector<Node *>

    // used for function
    char *name;
    Vector *args;  // Vector<Node *>
    Node *impl;

    int val;     // used only if kind=ND_NUM
    Type *ty;
    int offset;  // used only if kind=ND_LVAR
};

extern char *user_input;
extern Vector *code;
extern Map *locals;
extern Token *token;

void tokenize();

Token *consume(TokenKind kind, char *str);
void expect(char *op);
int expect_number();

void program();
Func *func();
Node *stmt();
Node *expr();
Node *assign();
Node *equality();
Node *relational();
Node *add();
Node *mul();
Node *unary();
Node *primary();

void gen_x86();
