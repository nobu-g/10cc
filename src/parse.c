#include "10cc.h"

Program *prog;               // The program
Func *fn;                    // The function being parsed
Node null_stmt = {ND_NULL};  // NOP node

void top_level();
void func();
Node *stmt();
Node *declaration();
Node *expr();
Node *assign();
Node *equality();
Node *relational();
Node *add();
Node *mul();
Node *unary();
Node *postfix();
Node *primary();

Node *new_node(NodeKind kind);
Node *new_node_binop(NodeKind kind, Node *lhs, Node *rhs);
Node *new_node_num(int val);
Node *new_node_lvar(Type *type, int offset);
Node *new_node_gvar(Type *type, char *name);
Node *new_node_func_call(Token *tok, Vector *args);

Type *new_ty(int ty, int size);
Type *int_ty();
Type *char_ty();
Type *ptr_to(Type *base);
Type *ary_of(Type *base, int size);
Type *read_type();

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
    Type *type = read_type();
    Token *tok = consume(TK_IDENT, NULL);
    if (!tok) {
        error_at(token->loc, "Invalid identifier");
    }
    if (consume(TK_RESERVED, "(")) {
        // function
        func(tok->str, type);
    } else {
        // global variable
        if (consume(TK_RESERVED, "[")) {
            type = ary_of(type, expect(TK_NUM, NULL)->val);
            expect(TK_RESERVED, "]");
        }

        Node *gvar = map_at(prog->gvars, tok->str);
        if (gvar) {
            error("グローバル変数 %s はすでに宣言されています", tok->str);
        } else {
            Node *node = new_node_gvar(type, tok->str);
            map_insert(prog->gvars, tok->str, node);
        }
        expect(TK_RESERVED, ";");
    }
}

void func(char *name, Type *ret_type) {
    /**
     * "int f(int a, int b) {}" の "(" まで読み終えた
     */
    fn = calloc(1, sizeof(Func));
    fn->name = name;
    fn->lvars = map_create();
    fn->args = vec_create();
    fn->ret_type = ret_type;
    while (!consume(TK_RESERVED, ")")) {
        if (fn->args->len > 0) {
            expect(TK_RESERVED, ",");
        }
        vec_push(fn->args, declaration());
    }
    map_insert(prog->fns, fn->name, fn);
    expect(TK_RESERVED, "{");
    fn->body = new_node(ND_BLOCK);
    fn->body->stmts = vec_create();
    while (!consume(TK_RESERVED, "}")) {
        vec_push(fn->body->stmts, stmt());
    }
}

/**
 * stmt = "{" stmt* "}"
 *      | "return" expr ";"
 *      | "if" "(" expr ")" stmt ("else" stmt)?
 *      | "while" "(" expr ")" stmt
 *      | "for" "(" expr? ";" expr? ";" expr? ")" stmt
 *      | declaration ";"
 *      | expr? ";"
 */
Node *stmt() {
    Node *node;
    if (consume(TK_RESERVED, "{")) {
        node = new_node(ND_BLOCK);
        node->stmts = vec_create();
        while (!consume(TK_RESERVED, "}")) {
            vec_push(node->stmts, stmt());
        }
        return node;
    } else if (consume(TK_RESERVED, "return")) {
        node = new_node(ND_RETURN);
        node->lhs = expr();
    } else if (consume(TK_RESERVED, "if")) {
        Node *node = new_node(ND_IF);
        expect(TK_RESERVED, "(");
        node->cond = expr();
        expect(TK_RESERVED, ")");
        node->then = stmt();
        if (consume(TK_RESERVED, "else")) {
            node->els = stmt();
        }
        return node;
    } else if (consume(TK_RESERVED, "while")) {
        Node *node = new_node(ND_WHILE);
        expect(TK_RESERVED, "(");
        node->cond = expr();
        expect(TK_RESERVED, ")");
        node->then = stmt();
        return node;
    } else if (consume(TK_RESERVED, "for")) {
        Node *node = new_node(ND_FOR);
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
        declaration();
        node = &null_stmt;
    } else if (consume(TK_RESERVED, ";")) {
        return &null_stmt;
    } else {
        node = expr();
    }
    if (!consume(TK_RESERVED, ";")) {
        error_at(token->loc, "';' expected ");
    }
    return node;
}

/*
 * declaration = T ident ("[" num? "]")?
 */
Node *declaration() {
    Type *type = read_type();
    Token *tok_ident = expect(TK_IDENT, NULL);
    if (map_at(fn->lvars, tok_ident->str)) {
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
    Node *node = new_node_lvar(type, get_offset(fn->lvars) + type->size);
    map_insert(fn->lvars, tok_ident->str, node);
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
        node = new_node_binop(ND_ASSIGN, node, assign());
    }
    return node;
}

/**
 * equality = relational (("==" | "!=") relational)?
 */
Node *equality() {
    Node *node = relational();
    if (consume(TK_RESERVED, "==")) {
        node = new_node_binop(ND_EQ, node, relational());
    } else if (consume(TK_RESERVED, "!=")) {
        node = new_node_binop(ND_NE, node, relational());
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
        return new_node_binop(ND_LE, node, add());
    } else if (consume(TK_RESERVED, ">=")) {
        return new_node_binop(ND_LE, add(), node);
    } else if (consume(TK_RESERVED, "<")) {
        return new_node_binop(ND_LT, node, add());
    } else if (consume(TK_RESERVED, ">")) {
        return new_node_binop(ND_LT, add(), node);
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
            node = new_node_binop(ND_ADD, node, mul());
        } else if (consume(TK_RESERVED, "-")) {
            node = new_node_binop(ND_SUB, node, mul());
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
            node = new_node_binop(ND_MUL, node, unary());
        } else if (consume(TK_RESERVED, "/")) {
            node = new_node_binop(ND_DIV, node, unary());
        } else {
            return node;
        }
    }
}

/*
 * unary = "sizeof" unary
 *       | ("+" | "-" | "&" | "*")? unary
 */
Node *unary() {
    if (consume(TK_RESERVED, "+")) {
        return unary();
    } else if (consume(TK_RESERVED, "-")) {
        return new_node_binop(ND_SUB, new_node_num(0), unary());
    } else if (consume(TK_RESERVED, "&")) {
        return new_node_binop(ND_ADDR, unary(), NULL);
    } else if (consume(TK_RESERVED, "*")) {
        return new_node_binop(ND_DEREF, unary(), NULL);
    } else if (consume(TK_RESERVED, "sizeof")) {
        // TODO: accept typename
        return new_node_binop(ND_SIZEOF, unary(), NULL);
    } else {
        return postfix();
    }
}

/**
 * postfix = primary
 *         | primary ("[" expr "]")+
 */
Node *postfix() {
    Node *node = primary();

    while(true) {
        if (consume(TK_RESERVED, "[")) {
            Node *sbsc = expr();
            expect(TK_RESERVED, "]");
            Node *sum = new_node_binop(ND_ADD, node, sbsc);
            node = new_node_binop(ND_DEREF, sum, NULL);
        } else {
            return node;
        }
    }
}

/**
 * primary = "(" expr ")"              // parenthesis
 *         | ident ("(" args ")")      // function call
 *         | ident                     // variable reference
 *         | num                       // immediate value
 */
Node *primary() {
    // "(" expr ")"
    if (consume(TK_RESERVED, "(")) {
        Node *node = expr();
        expect(TK_RESERVED, ")");
        return node;
    }

    Token *tok = consume(TK_IDENT, NULL);
    if (tok) {
        // function call
        if (consume(TK_RESERVED, "(")) {
            Vector *args = vec_create();
            while (!consume(TK_RESERVED, ")")) {
                if (args->len > 0) {
                    expect(TK_RESERVED, ",");
                }
                vec_push(args, expr());
            }
            return new_node_func_call(tok, args);
        }

        // variable reference
        Node *lvar = map_at(fn->lvars, tok->str);
        if (!lvar) {
            Node *gvar = map_at(prog->gvars, tok->str);
            if (!gvar) {
                error("undefined variable: '%s'", tok->str);
            }
            return new_node_gvar(gvar->ty, gvar->name);
        }
        return new_node_lvar(lvar->ty, lvar->offset);
    }

    return new_node_num(expect(TK_NUM, NULL)->val);;
}

Node *new_node(NodeKind kind) {
    Node *node = calloc(1, sizeof(Node));
    node->kind = kind;
    return node;
}

Node *new_node_binop(NodeKind kind, Node *lhs, Node *rhs) {
    Node *node = new_node(kind);
    node->lhs = lhs;
    node->rhs = rhs;
    return node;
}

Node *new_node_func_call(Token *tok, Vector *args) {
    Node *node = new_node(ND_FUNC_CALL);
    Func *fn = map_at(prog->fns, tok->str);
    if (!fn) {
        error_at(tok->loc, "Undefined function: '%s'", tok->str);
    }
    node->name = tok->str;
    node->args = args;
    node->ty = fn->ret_type;
    return node;
}

Node *new_node_lvar(Type *type, int offset) {
    Node *node = new_node(ND_LVAR);
    node->ty = type;
    node->offset = offset;
    return node;
}

Node *new_node_gvar(Type *type, char *name) {
    Node *node = new_node(ND_GVAR);
    node->ty = type;
    node->name = name;
    return node;
}

Node *new_node_num(int val) {
    Node *node = new_node(ND_NUM);
    node->val = val;
    node->ty = int_ty();
    return node;
}

/**
 * lvars を参照してこれまで確保されたスタック領域の総和を計算
 */
int get_offset(Map *lvars) {
    int offset = 0;
    for (int i = 0; i < lvars->len; i++) {
        Node *lvar = vec_get(lvars->vals, i);
        offset += lvar->ty->size;
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
