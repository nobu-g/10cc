#include "9cc.h"

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

char *user_input;
Token *token;
Node *code[100];
LVar *locals; // local varables

Node *new_node(NodeKind kind, Node *lhs, Node *rhs);
Node *new_node_num(int val);
void error_at(char *loc, char *fmt, ...);
LVar *find_lvar(Token *tok);

// returns True if the current token is op
bool consume(char *op) {
    if(token->kind != TK_RESERVED || strlen(op) != token->len ||
       memcmp(token->str, op, token->len) != 0) {
        return false;
    }
    token = token->next;
    return true;
}

bool consume_stmt(int kind) {
    if(token->kind != kind) {
        return false;
    }
    token = token->next;
    return true;
}

Token *consume_ident() {
    if(token->kind != TK_IDENT) {
        return NULL;
    }
    Token *tok = token;
    token = token->next;
    return tok;
}

void expect(char *op) {
    if(token->kind != TK_RESERVED || strlen(op) != token->len ||
       memcmp(token->str, op, token->len) != 0) {
        error_at(token->str, "'%s'ではありません", op);
    }
    token = token->next;
}

int expect_number() {
    if(token->kind != TK_NUM) {
        error_at(token->str, "数ではありません");
    }
    int val = token->val;
    token = token->next;
    return val;
}

bool at_eof() { return token->kind == TK_EOF; }

Token *new_token(TokenKind kind, Token *cur, char *str, int len) {
    Token *tok = calloc(1, sizeof(Token));
    tok->kind = kind;
    tok->str = str;
    tok->len = len;
    cur->next = tok;
    return tok;
}

bool startswith(char *p, char *q) { return memcmp(p, q, strlen(q)) == 0; }

bool is_alnum(char c) {
    return ('a' <= c && c <= 'z') || ('A' <= c && c <= 'Z') ||
           ('0' <= c && c <= '9') || (c == '_');
}

void tokenize() {
    Token head;
    head.next = NULL;
    Token *cur = &head;
    char *p = user_input;

    while(*p) {
        if(isspace(*p)) {
            p++;
            continue;
        }

        if(strncmp(p, "return", 6) == 0 && !is_alnum(p[6])) {
            cur = new_token(TK_RETURN, cur, p, 6);
            p += 6;
            continue;
        }

        if('a' <= *p && *p <= 'z') {
            int len = 1;
            while(('a' <= *(p + len) && *(p + len) <= 'z') ||
                  ('0' <= *(p + len) && *(p + len) <= '9')) {
                len++;
            }
            cur = new_token(TK_IDENT, cur, p, len);
            p += len;
            continue;
        }

        if(startswith(p, "==") || startswith(p, "!=") || startswith(p, "<=") ||
           startswith(p, ">=")) {
            cur = new_token(TK_RESERVED, cur, p, 2);
            p += 2;
            continue;
        }

        if(strchr("+-*/()<>=;", *p)) {
            cur = new_token(TK_RESERVED, cur, p, 1);
            p++;
            continue;
        }

        if(isdigit(*p)) {
            char *tmp = p;
            int val = strtol(p, &p, 10);
            cur = new_token(TK_NUM, cur, p, p - tmp);
            cur->val = val;
            continue;
        }
        error_at(p, "トークナイズできません");
    }

    new_token(TK_EOF, cur, p, 0);
    token = head.next;
}

void program() {
    LVar dummy = {NULL, "", 0, 0};
    locals = &dummy;
    int i = 0;
    while(!at_eof()) {
        code[i++] = stmt();
    }
    code[i] = NULL;
}

// Node *stmt() {
//     Node *node = expr();
//     expect(";");
//     return node;
// }
Node *stmt() {
    Node *node;
    if(consume_stmt(TK_RETURN)) {
        node = calloc(1, sizeof(Node));
        node->kind = ND_RETURN;
        node->lhs = expr();
    } else {
        node = expr();
    }
    expect(";");
    return node;
}

Node *expr() { return assign(); }

Node *assign() {
    Node *node = equality();
    if(consume("=")) {
        node = new_node(ND_ASSIGN, node, assign());
    }
    return node;
}

Node *equality() {
    Node *node = relational();
    if(consume("==")) {
        node = new_node(ND_EQ, node, relational());
    } else if(consume("!=")) {
        node = new_node(ND_NE, node, relational());
    } else {
        return node;
    }
}

Node *relational() {
    Node *node = add();
    if(consume("<=")) {
        return new_node(ND_LE, node, add());
    } else if(consume(">=")) {
        return new_node(ND_LE, add(), node);
    } else if(consume("<")) {
        return new_node(ND_LT, node, add());
    } else if(consume(">")) {
        return new_node(ND_LT, add(), node);
    } else {
        return node;
    }
}

Node *add() {
    Node *node = mul();
    for(;;) {
        if(consume("+")) {
            node = new_node(ND_ADD, node, mul());
        } else if(consume("-")) {
            node = new_node(ND_SUB, node, mul());
        } else {
            return node;
        }
    }
}

Node *mul() {
    Node *node = unary();
    for(;;) {
        if(consume("*")) {
            node = new_node(ND_MUL, node, unary());
        } else if(consume("/")) {
            node = new_node(ND_DIV, node, unary());
        } else {
            return node;
        }
    }
}

Node *unary() {
    if(consume("+")) {
        return primary();
    } else if(consume("-")) {
        return new_node(ND_SUB, new_node_num(0), primary());
    } else {
        return primary();
    }
}

Node *primary() {
    if(consume("(")) {
        Node *node = expr();
        expect(")");
        return node;
    }
    Token *tok = consume_ident();
    if(tok) {
        Node *node = calloc(1, sizeof(Node));
        node->kind = ND_LVAR;

        LVar *lvar = find_lvar(tok);
        if(lvar) {
            node->offset = lvar->offset;
        } else {
            lvar = calloc(1, sizeof(LVar));
            lvar->next = locals;
            lvar->name = tok->str;
            lvar->len = tok->len;
            lvar->offset = locals->offset + 8;
            node->offset = lvar->offset;
            locals = lvar;
        }
        return node;
    }
    return new_node_num(expect_number());
}

Node *new_node(NodeKind kind, Node *lhs, Node *rhs) {
    Node *node = calloc(1, sizeof(Node));
    node->kind = kind;
    node->lhs = lhs;
    node->rhs = rhs;
    return node;
}

Node *new_node_num(int val) {
    Node *node = calloc(1, sizeof(Node));
    node->kind = ND_NUM;
    node->val = val;
    return node;
}

void error_at(char *loc, char *fmt, ...) {
    va_list ap;
    va_start(ap, fmt);

    int pos = loc - user_input;
    fprintf(stderr, "%s\n", user_input);
    fprintf(stderr, "%*s", pos, ""); // pos個の空白を出力
    fprintf(stderr, "^ ");
    vfprintf(stderr, fmt, ap);
    fprintf(stderr, "\n");
    exit(1);
}

LVar *find_lvar(Token *tok) {
    for(LVar *var = locals; var; var = var->next) {
        if(var->len == tok->len && !memcmp(tok->str, var->name, var->len)) {
            return var;
        }
    }
    return NULL;
}
