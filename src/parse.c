#include "10cc.h"

Program *prog;  // The program
Func *f;        // The function being parsed

void top_level();
void func();
Node *stmt();
Node *expr();
Node *assign();
Node *equality();
Node *relational();
Node *add();
Node *mul();
Node *unary();
Node *primary();

Node *new_node(NodeKind kind, Node *lhs, Node *rhs);
Node *new_node_num(int val);
Node *new_node_lvar(Type *type, int offset);
Node *new_node_gvar(Type *type, char *name);
Node *new_node_func_call(char *name, Vector *args, Type *type);
void error_at(char *loc, char *fmt, ...);

bool at_eof();
Type *new_ty(int ty, int size);
Type *int_ty();
Type *char_ty();
Type *ptr_to(Type *base);
Type *ary_of(Type *base, int size);
Node *ary_to_ptr(Node *node);

Type *read_type();
Node *declaration();

Program *parse() {
    prog = calloc(1, sizeof(Program));
    prog->fns = map_create();
    prog->gvars = map_create();
    while (!at_eof()) {
        top_level();
    }
    return prog;
}

/**
 * program = (func|gvar)*
 */
void top_level() {
    while (!at_eof()) {
        Type *type = read_type();
        Token *tok = consume(TK_IDENT, NULL);
        if (!tok) {
            error_at(token->loc, "Invalid identifier");
        }
        if (consume(TK_RESERVED, "(")) {
            // 関数
            func(tok->str, type);
        } else {
            // グローバル変数
            if (map_at(prog->gvars, tok->str)) {
                error("グローバル変数 %s はすでに宣言されています", tok->str);
            }
            if (consume(TK_RESERVED, "[")) {
                type = ary_of(type, expect(TK_NUM, NULL)->val);
                expect(TK_RESERVED, "]");
            }
            Node *node = new_node_gvar(type, tok->str);
            map_insert(prog->gvars, tok->str, node);
            expect(TK_RESERVED, ";");
        }
    }
}

void func(char *name, Type *ret_type) {
    /**
     * "int f(int a, int b) {}" の "(" まで読み終えた
     */
    f = calloc(1, sizeof(Func));
    f->name = name;
    f->lvars = map_create();
    f->args = vec_create();
    f->ret_type = ret_type;
    while (!consume(TK_RESERVED, ")")) {
        if (f->args->len > 0) {
            expect(TK_RESERVED, ",");
        }
        vec_push(f->args, declaration());
    }
    map_insert(prog->fns, f->name, f);
    expect(TK_RESERVED, "{");
    f->body = vec_create();
    while (!consume(TK_RESERVED, "}")) {
        vec_push(f->body, stmt());
    }
}

/**
 * stmt = expr ";"
 *      | T ident ";"
 *      | "{" stmt* "}"
 *      | "return" expr ";"
 *      | "if" "(" expr ")" stmt ("else" stmt)?
 *      | "while" "(" expr ")" stmt
 *      | "for" "(" expr? ";" expr? ";" expr? ")" stmt
 */
Node *stmt() {
    Node *node;
    if (consume(TK_RESERVED, "{")) {
        node = calloc(1, sizeof(Node));
        node->kind = ND_BLOCK;
        node->stmts = vec_create();
        while (!consume(TK_RESERVED, "}")) {
            vec_push(node->stmts, (void *)stmt());
        }
        return node;
    } else if (consume(TK_RESERVED, "return")) {
        node = calloc(1, sizeof(Node));
        node->kind = ND_RETURN;
        node->lhs = expr();
    } else if (consume(TK_RESERVED, "if")) {
        node = calloc(1, sizeof(Node));
        node->kind = ND_IF;
        expect(TK_RESERVED, "(");
        node->cond = expr();
        expect(TK_RESERVED, ")");
        node->then = stmt();
        if (consume(TK_RESERVED, "else")) {
            node->els = stmt();
        }
        return node;
    } else if (consume(TK_RESERVED, "while")) {
        node = calloc(1, sizeof(Node));
        node->kind = ND_WHILE;
        expect(TK_RESERVED, "(");
        node->cond = expr();
        expect(TK_RESERVED, ")");
        node->then = stmt();
        return node;
    } else if (consume(TK_RESERVED, "for")) {
        node = calloc(1, sizeof(Node));
        node->kind = ND_FOR;
        expect(TK_RESERVED, "(");
        if (!consume(TK_RESERVED, ";")) {
            node->init = expr();
            expect(TK_RESERVED, ";");
        }
        if (!consume(TK_RESERVED, ";")) {
            node->cond = expr();
            expect(TK_RESERVED, ";");
        }
        if (!consume(TK_RESERVED, ";")) {
            node->upd = expr();
        }
        expect(TK_RESERVED, ")");
        node->then = stmt();
        return node;
    } else if (peek(TK_RESERVED, "int") || peek(TK_RESERVED, "char")) {
        node = declaration();
    } else {
        node = expr();
    }
    if (!consume(TK_RESERVED, ";")) {
        error_at(token->loc, "expected ';'");
    }
    return node;
}

/**
 * expr = assign
 */
Node *expr() { return assign(); }

/**
 * assign = equality ("=" assign)?
 */
Node *assign() {
    Node *node = equality();
    if (consume(TK_RESERVED, "=")) {
        node = new_node(ND_ASSIGN, node, assign());
    }
    return node;
}

/**
 * equality = relational (("==" | "!=") relational)?
 */
Node *equality() {
    Node *node = relational();
    if (consume(TK_RESERVED, "==")) {
        node = new_node(ND_EQ, node, relational());
    } else if (consume(TK_RESERVED, "!=")) {
        node = new_node(ND_NE, node, relational());
    } else {
        return node;
    }
}

/**
 * relational = add (("<=" | ">=" | "<" | ">") add)?
 */
Node *relational() {
    Node *node = add();
    if (consume(TK_RESERVED, "<=")) {
        return new_node(ND_LE, node, add());
    } else if (consume(TK_RESERVED, ">=")) {
        return new_node(ND_LE, add(), node);
    } else if (consume(TK_RESERVED, "<")) {
        return new_node(ND_LT, node, add());
    } else if (consume(TK_RESERVED, ">")) {
        return new_node(ND_LT, add(), node);
    } else {
        return node;
    }
}

/**
 * add = mul ("+" unary | "-" unary)*
 */
Node *add() {
    Node *node = mul();
    for (;;) {
        if (consume(TK_RESERVED, "+")) {
            Node *rhs;
            if (node->ty->ty == PTR || node->ty->ty == ARRAY) {
                int size = node->ty->ptr_to->size;
                rhs = new_node(ND_MUL, mul(), new_node_num(size));
            } else {
                rhs = mul();
            }
            node = new_node(ND_ADD, node, rhs);
        } else if (consume(TK_RESERVED, "-")) {
            Node *rhs;
            if (node->ty->ty == PTR || node->ty->ty == ARRAY) {
                int size = node->ty->ptr_to->size;
                rhs = new_node(ND_MUL, mul(), new_node_num(size));
            } else {
                rhs = mul();
            }
            node = new_node(ND_SUB, node, rhs);
        } else {
            return node;
        }
    }
}

/*
 * mul = unary (("*" | "/") unary)*
 */
Node *mul() {
    Node *node = unary();
    for (;;) {
        if (consume(TK_RESERVED, "*")) {
            node = new_node(ND_MUL, node, unary());
        } else if (consume(TK_RESERVED, "/")) {
            node = new_node(ND_DIV, node, unary());
        } else {
            return node;
        }
    }
}

/*
 * unary = "sizeof" unary
 *       | ("+" | "-" | "&" | "*")? primary
 */
Node *unary() {
    if (consume(TK_RESERVED, "+")) {
        return primary();
    } else if (consume(TK_RESERVED, "-")) {
        return new_node(ND_SUB, new_node_num(0), primary());
    } else if (consume(TK_RESERVED, "&")) {
        return new_node(ND_ADDR, unary(), NULL);
    } else if (consume(TK_RESERVED, "*")) {
        return new_node(ND_DEREF, unary(), NULL);
    } else if (consume(TK_RESERVED, "sizeof")) {
        return new_node_num(unary()->ty->size);
    } else {
        return primary();
    }
}

/**
 * primary = num              // 即値
 *         | ident ("(" ")")  // 変数参照 or 関数呼び出し
 *         | "(" expr ")"     // 括弧
 */
Node *primary() {
    Token *tok = consume(TK_IDENT, NULL);
    if (tok) {
        if (consume(TK_RESERVED, "(")) {
            // function call
            Vector *args = vec_create();
            for (;;) {
                if (consume(TK_RESERVED, ")")) {
                    break;
                }
                vec_push(args, expr());
                consume(TK_RESERVED, ",");
            }
            Func *f_called = map_at(prog->fns, tok->str);
            return new_node_func_call(tok->str, args, f_called->ret_type);
        } else if (consume(TK_RESERVED, "[")) {
            // variable reference
            Node *lhs = map_at(f->lvars, tok->str);
            if (!lhs) {
                lhs = map_at(prog->gvars, tok->str);
                if (!lhs) {
                    error("%sは未定義です", tok->str);
                }
            }
            Node *rhs = new_node(ND_MUL, expr(), new_node_num(lhs->ty->ptr_to->size));
            expect(TK_RESERVED, "]");
            Node *sum = new_node(ND_ADD, lhs, rhs);
            return new_node(ND_DEREF, sum, NULL);
        } else {
            Node *lvar = map_at(f->lvars, tok->str);
            if (!lvar) {
                Node *gvar = map_at(prog->gvars, tok->str);
                if (!gvar) {
                    error("%sは未定義です", tok->str);
                }
                return new_node_gvar(gvar->ty, gvar->name);
            }
            return new_node_lvar(lvar->ty, lvar->offset);
        }
    }
    if (consume(TK_RESERVED, "(")) {
        Node *node = expr();
        expect(TK_RESERVED, ")");
        return node;
    }

    Node *node = new_node_num(expect(TK_NUM, NULL)->val);
    if (consume(TK_RESERVED, "[")) {
        Node *lhs, *rhs;
        tok = consume(TK_IDENT, NULL);
        if (tok) {
            lhs = map_at(f->lvars, tok->str);
            if (!lhs) {
                error("%sは未定義です", tok->str);
            }
            if (lhs->ty->ty == ARRAY || lhs->ty->ty == PTR) {
                rhs = new_node(ND_MUL, node, new_node_num(lhs->ty->ptr_to->size));
            } else {
                rhs = node;  // deref で死ぬ†運命[さだめ]†
            }
        } else {
            lhs = new_node_num(expect(TK_NUM, NULL)->val);
            rhs = node;  // deref で死ぬ†運命[さだめ]†
        }
        expect(TK_RESERVED, "]");
        Node *sum = new_node(ND_ADD, lhs, rhs);
        return new_node(ND_DEREF, sum, NULL);
    }
    return node;
}

Node *new_node(NodeKind kind, Node *lhs, Node *rhs) {
    Node *node = calloc(1, sizeof(Node));
    node->kind = kind;
    node->lhs = lhs;
    node->rhs = rhs;

    if (node->lhs && (node->lhs->kind == ND_LVAR || node->lhs->kind == ND_GVAR) && node->lhs->ty->ty == ARRAY) {
        node->lhs = ary_to_ptr(node->lhs);
    }
    if (node->rhs && (node->lhs->kind == ND_LVAR || node->lhs->kind == ND_GVAR) && node->rhs->ty->ty == ARRAY) {
        node->rhs = ary_to_ptr(node->rhs);
    }

    switch (kind) {
    case ND_ADD:
    case ND_SUB:
    case ND_MUL:
    case ND_DIV:
    case ND_EQ:
    case ND_NE:
    case ND_LE:
    case ND_LT:
    case ND_ASSIGN:
        node->ty = lhs->ty;
        break;
    case ND_ADDR:
        node->ty = ptr_to(lhs->ty);
        break;
    case ND_DEREF:
        node->ty = node->lhs->ty->ptr_to;
        break;
    default:
        break;
    }
#if DEBUG <= 1
    fprintf(stderr, "ND_%d\n", node->kind);
#endif
    return node;
}

Node *new_node_func_call(char *name, Vector *args, Type *type) {
    Node *node = calloc(1, sizeof(Node));
    node->kind = ND_FUNC_CALL;
    node->name = name;
    node->args = args;
    node->ty = type;
#if DEBUG <= 1
    fprintf(stderr, "ND_FUNC_CALL\n");
#endif
    return node;
}

Node *new_node_lvar(Type *type, int offset) {
    Node *node = calloc(1, sizeof(Node));
    node->kind = ND_LVAR;
    node->ty = type;
    node->offset = offset;
#if DEBUG <= 1
    fprintf(stderr, "ND_LVAR\n");
#endif
    return node;
}

Node *new_node_gvar(Type *type, char *name) {
    Node *node = calloc(1, sizeof(Node));
    node->kind = ND_GVAR;
    node->name = name;
    node->ty = type;
#if DEBUG <= 1
    fprintf(stderr, "ND_GVAR\n");
#endif
    return node;
}

Node *new_node_num(int val) {
    Node *node = calloc(1, sizeof(Node));
    node->kind = ND_NUM;
    node->val = val;
    node->ty = new_ty(INT, 4);
#if DEBUG <= 1
    fprintf(stderr, "ND_NUM\n");
#endif
    return node;
}

bool at_eof() { return token->kind == TK_EOF; }

/**
 * lvars を参照してこれまで確保されたスタック領域の総和を計算
 */
int get_offset(Map *lvars) {
    int offset = 0;
    for (int i = 0; i < lvars->len; i++) {
        Node *node = lvars->vals->data[i];
        offset += node->ty->size;
    }
    return offset;
}

Type *new_ty(int ty, int size) {
    Type *ret = calloc(1, sizeof(Type));
    ret->ty = ty;
    ret->size = size;
    return ret;
}

Type *int_ty() { return new_ty(INT, 4); }

Type *char_ty() { return new_ty(CHAR, 1); }

Type *ptr_to(Type *dest) {
    Type *ty = new_ty(PTR, 8);
    ty->ptr_to = dest;
    return ty;
}

Type *ary_of(Type *base, int size) {
    Type *ty = new_ty(ARRAY, base->size * size);
    ty->ptr_to = base;
    ty->array_size = size;
    return ty;
}

Node *ary_to_ptr(Node *base) {
    if (base->ty->ty != ARRAY) {
        error("配列ではありません");
    }

    // &(base[0])
    Node *addr = calloc(1, sizeof(Node));
    addr->kind = ND_ADDR;
    addr->lhs = base;
    addr->ty = ptr_to(base->ty->ptr_to);

    return addr;
}

// 型を読み込んでそれを返す
Type *read_type() {
    Type *ty;
    if (consume(TK_RESERVED, "int")) {
        ty = int_ty();
    } else if (consume(TK_RESERVED, "char")) {
        ty = char_ty();
    } else {
        error_at(token->loc, "Invalid type");
    }
    while (consume(TK_RESERVED, "*")) {
        ty = ptr_to(ty);
    }
    return ty;
}

/*
 * declaration = T ident ("[" num? "]")?
 */
Node *declaration() {
    Type *type = read_type();
    Token *tok_ident = expect(TK_IDENT, NULL);
    if (map_at(f->lvars, tok_ident->str)) {
        error("Redefinition of '%s'", tok_ident->str);
    }
    if (consume(TK_RESERVED, "[")) {
        Token *token = consume(TK_NUM, NULL);
        if (token) {
            type = ary_of(type, token->val);
        } else {
            type = ary_of(type, 0);  // tentatively, array length is 0
        }
        expect(TK_RESERVED, "]");
    }
    Node *node = new_node_lvar(type, get_offset(f->lvars) + type->size);
    map_insert(f->lvars, tok_ident->str, node);
    return node;
}
