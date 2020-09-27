#include "10cc.h"

Program *prog;               // The program
Func *fn;                    // The function being parsed
Node null_stmt = {ND_NULL};  // NOP node

void top_level();
void func();
Node *stmt();
LVar *declaration();
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
Node *new_node_uniop(NodeKind kind, Node *lhs);
Node *new_node_binop(NodeKind kind, Node *lhs, Node *rhs);
Node *new_node_num(int val);
Node *new_node_lvar(LVar *lvar);
Node *new_node_gvar(GVar *gvar);
Node *new_node_func_call(Token *tok, Vector *args);

Type *new_ty(TypeKind kind, int size);
Type *int_ty();
Type *char_ty();
Type *ptr_to(Type *base);
Type *ary_of(Type *base, int size);
Type *read_type();

/**
 * program = top_level* EOF
 */
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
 * top_level = T IDENT (func | ("[" NUM "]")? ";")
 */
void top_level() {
    Type *type = read_type();
    Token *tok = expect(TK_IDENT, NULL);
    if (consume(TK_RESERVED, "(")) {
        // function
        func(tok->str, type);
    } else {
        // global variable
        if (consume(TK_RESERVED, "[")) {
            type = ary_of(type, expect(TK_NUM, NULL)->val);
            expect(TK_RESERVED, "]");
        }
        GVar *gvar = map_at(prog->gvars, tok->str);
        if (gvar) {
            if (!same_type(type, gvar->type)) {
                error_at(token->loc, "Redefinition of '%s' with a different type", tok->str);
            }
        } else {
            gvar = calloc(1, sizeof(GVar));
            gvar->name = tok->str;
            gvar->type = type;
            map_insert(prog->gvars, gvar->name, gvar);
        }
        expect(TK_RESERVED, ";");
    }
}
/**
 * func = (declaration ("," declaration)*)? ")" "{" stmt* "}"
 */
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
 *      | ";"
 *      | expr ";"
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
    } else if (at_typename()) {
        declaration();
        node = &null_stmt;
    } else if (consume(TK_RESERVED, ";")) {
        return &null_stmt;
    } else {
        node = expr();
    }
    expect(TK_RESERVED, ";");
    return node;
}

/*
 * declaration = T IDENT ("[" NUM? "]")?
 */
LVar *declaration() {
    Type *type = read_type();
    Token *tok = expect(TK_IDENT, NULL);
    if (consume(TK_RESERVED, "[")) {
        Token *tok_num = consume(TK_NUM, NULL);
        int array_size = tok_num ? tok_num->val : 0;  // tentatively, array size is 0 when omitted
        expect(TK_RESERVED, "]");
        type = ary_of(type, array_size);
    }
    LVar *lvar = map_at(fn->lvars, tok->str);
    if (lvar) {
        if (!same_type(type, lvar->type)) {
            error_at(token->loc, "Redefinition of '%s' with a different type", tok->str);
        }
    } else {
        lvar = calloc(1, sizeof(LVar));
        lvar->name = tok->str;
        lvar->type = type;
        map_insert(fn->lvars, lvar->name, lvar);
    }
    return lvar;
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
    }
    return node;
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
 * unary = ("+" | "-" | "&" | "*") unary
 *       | "sizeof" "(" (T|expr) ")"
 *       | "sizeof" unary
 *       | postfix
 */
Node *unary() {
    if (consume(TK_RESERVED, "+")) {
        return unary();
    } else if (consume(TK_RESERVED, "-")) {
        return new_node_binop(ND_SUB, new_node_num(0), unary());
    } else if (consume(TK_RESERVED, "&")) {
        return new_node_uniop(ND_ADDR, unary());
    } else if (consume(TK_RESERVED, "*")) {
        return new_node_uniop(ND_DEREF, unary());
    } else if (consume(TK_RESERVED, "sizeof")) {
        if (consume(TK_RESERVED, "(")) {
            Node *node;
            if (at_typename()) {
                node = new_node_num(read_type()->size);
            } else {
                node = new_node_uniop(ND_SIZEOF, expr());
            }
            expect(TK_RESERVED, ")");
            return node;
        }
        return new_node_uniop(ND_SIZEOF, unary());
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

    while (true) {
        // x[y] -> *(x + y)
        if (consume(TK_RESERVED, "[")) {
            Node *sbsc = expr();
            expect(TK_RESERVED, "]");
            Node *sum = new_node_binop(ND_ADD, node, sbsc);
            node = new_node_uniop(ND_DEREF, sum);
        } else {
            return node;
        }
    }
}

/**
 * primary = "(" expr ")"                       // parenthesis
 *         | IDENT "(" (expr ("," expr)*)? ")"  // function call
 *         | IDENT                              // variable reference
 *         | NUM                                // immediate value
 */
Node *primary() {
    // parenthesis
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
        LVar *lvar = map_at(fn->lvars, tok->str);
        if (!lvar) {
            GVar *gvar = map_at(prog->gvars, tok->str);
            if (!gvar) {
                error_at(token->loc, "undefined variable: '%s'", tok->str);
            }
            return new_node_gvar(gvar);
        }
        return new_node_lvar(lvar);
    }
    // immediate value
    return new_node_num(expect(TK_NUM, NULL)->val);
}

Node *new_node(NodeKind kind) {
    Node *node = calloc(1, sizeof(Node));
    node->kind = kind;
    return node;
}

Node *new_node_uniop(NodeKind kind, Node *lhs) {
    Node *node = new_node(kind);
    node->lhs = lhs;
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
    node->func = fn;
    node->args = args;
    return node;
}

Node *new_node_lvar(LVar *lvar) {
    Node *node = new_node(ND_LVAR);
    node->lvar = lvar;
    return node;
}

Node *new_node_gvar(GVar *gvar) {
    Node *node = new_node(ND_GVAR);
    node->gvar = gvar;
    return node;
}

Node *new_node_num(int val) {
    Node *node = new_node(ND_NUM);
    node->val = val;
    node->type = int_ty();
    return node;
}

Type *new_ty(TypeKind kind, int size) {
    Type *ret = calloc(1, sizeof(Type));
    ret->kind = kind;
    ret->size = size;
    return ret;
}

Type *int_ty() { return new_ty(TY_INT, 4); }

Type *char_ty() { return new_ty(TY_CHAR, 1); }

Type *ptr_to(Type *dest) {
    Type *type = new_ty(TY_PTR, 8);
    type->ptr_to = dest;
    return type;
}

Type *ary_of(Type *base, int size) {
    Type *type = new_ty(TY_ARRAY, base->size * size);
    type->ptr_to = base;
    type->array_size = size;
    return type;
}

Type *read_type() {
    Type *type;
    if (consume(TK_RESERVED, "int")) {
        type = int_ty();
    } else if (consume(TK_RESERVED, "char")) {
        type = char_ty();
    } else {
        error_at(token->loc, "Invalid type");
    }
    while (consume(TK_RESERVED, "*")) {
        type = ptr_to(type);
    }
    return type;
}
