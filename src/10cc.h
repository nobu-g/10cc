#include <ctype.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// 0: TRACE
// 1: DEBUG
// 2: INFO
// 3: WARNING
// 4: ERROR
// 5: CRITICAL
// 6: NONE
#define DEBUG 6

/*
 * container.c
 */
typedef struct {
    void **data;
    int capacity;
    int len;
} Vector;

Vector *vec_create();
void vec_push(Vector *vec, void *elem);
void *vec_get(Vector *vec, int index);

typedef struct {
    Vector *keys;
    Vector *vals;
    int len;
} Map;

Map *map_create();
void map_insert(Map *map, char *key, void *val);
void *map_at(Map *map, char *key);

typedef enum {
    TK_RESERVED,  // operators and reserved words
    TK_IDENT,     // identifier
    TK_NUM,       // number
    TK_EOF,       // end of file
} TokenKind;

typedef struct Token Token;

struct Token {
    TokenKind kind;
    Token *next;
    int val;
    char *str;
    char *loc;
};

typedef struct Type Type;

struct Type {
    enum { INT, CHAR, PTR, ARRAY } ty;  // ->tyでアクセスするとINT: 0, PTR: 1, ARRAY: 2
    int size;                           // INT: 4, PTR: 8, ARRAY: ptr_to->size * array_size
    struct Type *ptr_to;
    int array_size;  // 配列の要素数
};

typedef struct {
    char *name;      // 関数の名前
    Map *lvars;      // ローカル変数(args含む) (Map<char *, Node *>)
    Vector *args;    // 引数 (Vector<Node *>)
    Vector *body;    // 実装 (Vector<Node *>)
    Type *ret_type;  // 戻り値の型
} Func;

typedef struct {
    Map *fns;
    Map *gvars;
} Program;

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
    ND_WHILE,      // while
    ND_FOR,        // for
    ND_BLOCK,      // block
    ND_FUNC_CALL,  // function call
    ND_LVAR,       // local variable
    ND_GVAR,       // global variable
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
    Vector *args;  // Vector<Node *>
    Node *impl;

    char *name;  // used only if kind in (ND_FUNC, ND_GVAR)
    int val;     // used only if kind=ND_NUM
    Type *ty;    // used only if kind in (ND_NUM, ND_LVAR, ND_GVAR)
    int offset;  // used only if kind=ND_LVAR
};

/*
 * tokenize.c
 */
extern char *user_input;
extern Token *token;

void tokenize();
Token *peek(TokenKind kind, char *str);
Token *consume(TokenKind kind, char *str);
Token *expect(TokenKind kind, char *str);
bool at_eof();
/*
 * parse.c
 */
Program *parse();
int get_offset(Map *lvars);

/*
 * sema.c
 */
void sema(Program *prog);

/*
 * codegen.c
 */
void gen_x86_64(Program *prog);

/*
 * utils.c
 */
void error(char *fmt, ...);
void error_at(char *loc, char *fmt, ...);
void draw_node_tree(Node *node, int depth, char *prefix);
void draw_ast();
