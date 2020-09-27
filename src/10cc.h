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

typedef struct Token Token;
typedef struct Node Node;
typedef struct Type Type;
typedef struct Func Func;

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
void *vec_set(Vector *vec, int index, void *elem);

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

struct Token {
    TokenKind kind;
    Token *next;
    int val;
    char *str;
    char *loc;
};

typedef enum {
    TY_INT,
    TY_CHAR,
    TY_PTR,
    TY_ARRAY,
} TypeKind;

struct Type {
    TypeKind kind;
    int size;  // CHAR: 1, INT: 4, PTR: 8, ARRAY: ptr_to->size * array_size
    Type *ptr_to;
    int array_size;  // number of array elements
    char *str;  // string which represents this type
};

typedef struct LVar {
    char *name;
    Type *type;
    int offset;  // offset from RBP
} LVar;

typedef struct GVar {
    char *name;
    Type *type;
} GVar;

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
    ND_NUM,        // immediate value
    ND_ADDR,       // unary &
    ND_DEREF,      // unary *
    ND_SIZEOF,     // sizeof
    ND_NULL        // NOP
} NodeKind;

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

    // used for function call
    Func *func;
    Vector *args;  // Vector<Node *>

    int val;     // used only if kind = ND_NUM
    Type *type;  // used only if node is expr
    LVar *lvar;  // used only if kind = ND_LVAR
    GVar *gvar;  // used only if kind = ND_GVAR
};

struct Func {
    char *name;      // name of function
    Map *lvars;      // local variables (including arguments) (Map<char *, LVar *>)
    Vector *args;    // arguments (Vector<LVar *>)
    Node *body;      // implementation
    Type *ret_type;  // type of return value
};

typedef struct {
    Map *fns;    // function definitions (Map<char *, Func *>)
    Map *gvars;  // global variable declarations (Map<char *, GVar *>)
} Program;

/*
 * tokenize.c
 */
extern char *user_input;
extern Token *token;

void tokenize();
Token *peek(TokenKind kind, char *str);
Token *consume(TokenKind kind, char *str);
Token *expect(TokenKind kind, char *str);
bool at_typename();
bool at_eof();

/*
 * parse.c
 */
Program *parse();
Node *new_node_uniop(NodeKind kind, Node *lhs);
Node *new_node_binop(NodeKind kind, Node *lhs, Node *rhs);
Node *new_node_num(int val);
Type *ptr_to(Type *base);
Type *int_ty();

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
void assert(bool cond, char *fmt, ...);
void draw_ast();
bool same_type(Type *x, Type *y);
