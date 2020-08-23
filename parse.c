#include "10cc.h"

Func *f;
Map *funcs; // Map[char *, Func]
Map *gvars; // Map[char *, Node]

Node *new_node(NodeKind kind, Node *lhs, Node *rhs);
Node *new_node_num(int val);
Node *new_node_lvar(Type *type, int offset);
Node *new_node_gvar(Type *type, char *name);
Node *new_node_func_call(char *name, Vector *args, Type *type);
void error_at(char *loc, char *fmt, ...);

bool at_eof();
int get_offset(Type *type, Map *lvars);
Node *ary_to_ptr(Node *node);

Type *new_ty(int ty, int size);
Type *int_ty();
Type *ptr_to(Type *base);
Type *ary_of(Type *base, int size);

/**
 * program = (func|gvar)*
 */
void program() {
    funcs = create_map();
    gvars = create_map();
    while (!at_eof()) {
        Token *tok = consume(TK_INT, NULL);
        if (!tok) {
            fprintf(stderr, "tok: %s", token->str);
            error("有効な型ではありません");
        }
        Type *type = int_ty();
        while (consume(TK_RESERVED, "*")) {
            type = ptr_to(type);
        }
        tok = consume(TK_IDENT, NULL);
        if (!tok) {
            error("識別子ではありません");
        }

        if (consume(TK_RESERVED, "(")) {
            // 関数
            func(tok->str, type);
        } else {
            // グローバル変数
            if (get_elem_from_map(gvars, tok->str)) {
                error("グローバル変数 %s はすでに宣言されています", tok->str);
            }

            if (consume(TK_RESERVED, "[")) {
                type = ary_of(type, expect_number());
                expect("]");
            }

            Node *node = new_node_gvar(type, tok->str);
            add_elem_to_map(gvars, tok->str, node);
            expect(";");
        }
    }
}

void func(char *name, Type *ret_type) {
    /**
     * "int f(int a, int b) {}" の "(" まで読み終えた
     */
    f = calloc(1, sizeof(Func));
    f->name = name;
    f->lvars = create_map();
    f->args = create_vector();
    f->ret_type = ret_type;
    for (;;) {
        Token *tok = consume(TK_INT, NULL);
        if (tok) {
            Type *arg_type = int_ty();
            while (consume(TK_RESERVED, "*")) {
                arg_type = ptr_to(arg_type);
            }
            tok = consume(TK_IDENT, NULL);
            if (!tok) {
                error("変数名ではありません");
            }
            if (get_elem_from_map(f->lvars, tok->str)) {
                error("変数 %s はすでに宣言されています", tok->str);
            }
            Node *node = new_node_lvar(arg_type, get_offset(arg_type, f->lvars));
            push(f->args, node);
            add_elem_to_map(f->lvars, tok->str, node);
            if (!consume(TK_RESERVED, ",")) {
                break;
            }
        } else {
            break;
        }
    }
    expect(")");
    add_elem_to_map(funcs, f->name, f);
    expect("{");
    f->body = create_vector();
    while (!consume(TK_RESERVED, "}")) {
        push(f->body, stmt());
    }
}

/**
 * stmt = expr ";"
 *      | "{" stmt* "}"
 *      | "if" "(" expr ")" stmt ("else" stmt)?
 *      | "while" "(" expr ")" stmt
 *      | "for" "(" expr? ";" expr? ";" expr? ")" stmt
 *      | "return" expr ";"
 */
Node *stmt() {
    Node *node;
    if (consume(TK_RESERVED, "{")) {
        node = calloc(1, sizeof(Node));
        node->kind = ND_BLOCK;
        node->stmts = create_vector();
        while (!consume(TK_RESERVED, "}")) {
            push(node->stmts, (void *)stmt());
        }
        return node;
    } else if (consume(TK_RETURN, NULL)) {
        node = calloc(1, sizeof(Node));
        node->kind = ND_RETURN;
        node->lhs = expr();
    } else if (consume(TK_IF, NULL)) {
        node = calloc(1, sizeof(Node));
        node->kind = ND_IF;
        expect("(");
        node->cond = expr();
        expect(")");
        node->then = stmt();
        if (consume(TK_ELSE, NULL)) {
            node->els = stmt();
        }
        return node;
    } else if (consume(TK_WHILE, NULL)) {
        node = calloc(1, sizeof(Node));
        node->kind = ND_WHILE;
        expect("(");
        node->cond = expr();
        expect(")");
        node->then = stmt();
        return node;
    } else if (consume(TK_FOR, NULL)) {
        node = calloc(1, sizeof(Node));
        node->kind = ND_FOR;
        expect("(");
        if (!consume(TK_RESERVED, ";")) {
            node->init = expr();
            expect(";");
        }
        if (!consume(TK_RESERVED, ";")) {
            node->cond = expr();
            expect(";");
        }
        if (!consume(TK_RESERVED, ";")) {
            node->upd = expr();
        }
        expect(")");
        node->then = stmt();
        return node;
    } else {
        node = expr();
    }
    if (!consume(TK_RESERVED, ";")) {
        error_at(token->loc, "';'ではないトークンです");
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
    } else if (consume(TK_SIZEOF, NULL)) {
        return new_node_num(unary()->ty->size);
    } else {
        return primary();
    }
}

/**
 * primary = num              // 即値
 *         | ident ("(" ")")  // 変数参照 or 関数呼び出し
 *         | "int" ident      // 変数宣言
 *         | "(" expr ")"     // 括弧
 */
Node *primary() {
    // 変数宣言
    Token *tok = consume(TK_INT, NULL);
    if (tok) {
        Type *type = int_ty();
        while (consume(TK_RESERVED, "*")) {
            type = ptr_to(type);
        }
        tok = consume(TK_IDENT, NULL);
        if (!tok) {
            error("変数名ではありません");
        }
        if (get_elem_from_map(f->lvars, tok->str)) {
            error("変数 %s はすでに宣言されています", tok->str);
        }
        if (consume(TK_RESERVED, "[")) {
            type = ary_of(type, expect_number());
            expect("]");
        }
        Node *node = new_node_lvar(type, get_offset(type, f->lvars));
        add_elem_to_map(f->lvars, tok->str, node);
        return node;
    }

    // 変数参照 or 関数呼び出し
    tok = consume(TK_IDENT, NULL);
    if (tok) {
        // 関数呼び出し
        if (consume(TK_RESERVED, "(")) {
            Vector *args = create_vector();
            for (;;) {
                if (consume(TK_RESERVED, ")")) {
                    break;
                }
                push(args, expr());
                consume(TK_RESERVED, ",");
            }
            Func *f_called = get_elem_from_map(funcs, tok->str);
            return new_node_func_call(tok->str, args, f_called->ret_type);
            // 変数参照
        } else if (consume(TK_RESERVED, "[")) {
            Node *lhs = get_elem_from_map(f->lvars, tok->str);
            if (!lhs) {
                lhs = get_elem_from_map(gvars, tok->str);
                if (!lhs) {
                    error("%sは未定義です", tok->str);
                }
            }
            Node *rhs = new_node(ND_MUL, expr(), new_node_num(lhs->ty->ptr_to->size));
            expect("]");
            Node *sum = new_node(ND_ADD, lhs, rhs);
            return new_node(ND_DEREF, sum, NULL);
        } else {
            Node *lvar = get_elem_from_map(f->lvars, tok->str);
            if (!lvar) {
                Node *gvar = get_elem_from_map(gvars, tok->str);
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
        expect(")");
        return node;
    }

    Node *node = new_node_num(expect_number());
    if (consume(TK_RESERVED, "[")) {
        tok = consume(TK_IDENT, NULL);
        Node *lhs, *rhs;
        if (tok) {
            lhs = get_elem_from_map(f->lvars, tok->str);
            if (!lhs) {
                error("%sは未定義です", tok->str);
            }
            if (lhs->ty->ty == ARRAY || lhs->ty->ty == PTR) {
                rhs = new_node(ND_MUL, node, new_node_num(lhs->ty->ptr_to->size));
            } else {
                rhs = node; // deref で死ぬ†運命[さだめ]†
            }
        } else {
            lhs = new_node_num(expect_number());
            rhs = node; // deref で死ぬ†運命[さだめ]†
        }
        expect("]");
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
 * lvars を参照してこれまで確保されたスタック領域の和を計算し，
 * type 型の新たな変数を格納できるだけの offset を返す
 */
int get_offset(Type *type, Map *lvars) {
    int offset = 0;
    for (int i = 0; i < lvars->len; i++) {
        Node *node = lvars->vals->data[i];
        if (node->ty->ty == ARRAY) {
            offset += node->ty->array_size * 8;
        } else {
            offset += 8;
        }
    }
    if (type->ty == ARRAY) {
        offset += 8;
    } else {
        offset += 8;
    }
    return offset;
}

/**
 * base: ARRAY を表すノード
 */
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

Type *new_ty(int ty, int size) {
    Type *ret = calloc(1, sizeof(Type));
    ret->ty = ty;
    ret->size = size;
    return ret;
}

Type *int_ty() { return new_ty(INT, 4); }

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
