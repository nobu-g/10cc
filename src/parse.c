#include "10cc.h"

Program *prog;               // The program
Scope *scope;                // Current scope
Node null_stmt = {ND_NULL};  // NOP node

void top_level();
LVar *param_declaration();
void declaration();
Func *add_func(Type *ret_type, char *name, Vector *args);
LVar *add_lvar(Type *type, char *name);
GVar *add_gvar(Type *type, char *name);
Node *stmt();
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

Type *new_ty(TypeKind kind, int size, char *repr);
Type *int_ty();
Type *char_ty();
Type *ptr_to(Type *base);
Type *ary_of(Type *base, int size);
Type *read_type();
Type *read_array(Type *base);

Scope *new_scope() {
    Scope *sc = calloc(1, sizeof(Scope));
    sc->parent = NULL;
    sc->children = vec_create();
    sc->lvars = map_create();
    return sc;
}

void enter_scope() {
    Scope *sc = new_scope();
    sc->parent = scope;
    vec_push(scope->children, sc);
    scope = sc;
}

void leave_scope() { scope = scope->parent; }

/**
 * program = top_level* EOF
 */
Program *parse() {
    prog = calloc(1, sizeof(Program));
    prog->funcs = map_create();
    prog->gvars = map_create();
    while (!at_eof()) {
        top_level();
    }
    return prog;
}

/**
 * top_level = T IDENT (
 *             "(" (param_declaration ("," param_declaration)*)? ")" (";" | "{" stmt* "}")
 *             | ("[" NUM? "]")* ";"
 *           )
 */
void top_level() {
    Type *type = read_type();
    Token *tok = expect(TK_IDENT, NULL);
    if (consume(TK_RESERVED, "(")) {
        // function declaration/definintion
        scope = new_scope();
        // parse arguments
        Vector *args = vec_create();
        while (!consume(TK_RESERVED, ")")) {
            if (args->len > 0) {
                expect(TK_RESERVED, ",");
            }
            vec_push(args, param_declaration());
        }

        Func *func = add_func(type, tok->str, args);

        if (consume(TK_RESERVED, ";")) {
            // prototype declaration
            func->body = NULL;
            return;
        }

        // function definition
        expect(TK_RESERVED, "{");
        if (func->body) {
            error_at(token->loc, "Redefinition of function: '%s'", func->name);
        }
        func->body = new_node(ND_BLOCK);
        func->body->stmts = vec_create();
        while (!consume(TK_RESERVED, "}")) {
            vec_push(func->body->stmts, stmt());
        }
    } else {
        // global variable
        type = read_array(type);
        expect(TK_RESERVED, ";");
        add_gvar(type, tok->str);
    }
}

/*
 * param_declaration = T IDENT ("[" NUM? "]")*
 */
LVar *param_declaration() {
    Type *type = read_type();
    Token *tok = expect(TK_IDENT, NULL);
    type = read_array(type);
    return add_lvar(type, tok->str);
}

/*
 * declaration = T IDENT ("[" NUM? "]")* ";"
 */
void declaration() {
    Type *type = read_type();
    Token *tok = expect(TK_IDENT, NULL);
    type = read_array(type);
    add_lvar(type, tok->str);
    expect(TK_RESERVED, ";");
}

Func *add_func(Type *ret_type, char *name, Vector *args) {
    Func *fn = map_at(prog->funcs, name);
    if (fn) {
        bool is_compatible = same_type(ret_type, fn->ret_type) && (args->len == fn->args->len);
        if (is_compatible) {
            for (int i = 0; i < args->len; i++) {
                LVar *arg = vec_get(args, i);
                LVar *fn_arg = vec_get(fn->args, i);
                if (!same_type(arg->type, fn_arg->type)) {
                    is_compatible = false;
                }
            }
        }
        if (!is_compatible) {
            error_at(token->loc, "Incompatible function declaration");
        }
        if (fn->body) {
            return fn;  // when function definition preceeds function declaration
        }
    }
    fn = calloc(1, sizeof(Func));
    fn->name = name;
    fn->args = args;
    fn->ret_type = ret_type;
    fn->scope = scope;
    map_insert(prog->funcs, name, fn);
    return fn;
}

LVar *find_lvar(char *name) {
    LVar *lvar;
    Scope *sc = scope;
    while (sc) {
        lvar = map_at(sc->lvars, name);
        if (lvar) {
            break;
        }
        sc = sc->parent;
    }
    return lvar;
}

GVar *find_gvar(char *name) {
    GVar *gvar = map_at(prog->gvars, name);
    return gvar;
}

LVar *add_lvar(Type *type, char *name) {
    LVar *lvar = map_at(scope->lvars, name);  // search within current scope
    if (lvar) {
        if (!same_type(type, lvar->type)) {
            error_at(token->loc, "Redefinition of '%s' with a different type: '%s' vs '%s'", name, type->str,
                     lvar->type->str);
        }
    } else {
        lvar = calloc(1, sizeof(LVar));
        lvar->name = name;
        lvar->type = type;
        map_insert(scope->lvars, name, lvar);
    }
    return lvar;
}

GVar *add_gvar(Type *type, char *name) {
    GVar *gvar = map_at(prog->gvars, name);
    if (gvar) {
        if (!same_type(type, gvar->type)) {
            error_at(token->loc, "Redefinition of '%s' with a different type: '%s' vs '%s'", name, type->str,
                     gvar->type->str);
        }
    } else {
        gvar = calloc(1, sizeof(GVar));
        gvar->name = name;
        gvar->type = type;
        map_insert(prog->gvars, name, gvar);
    }
    return gvar;
}

Type *read_array(Type *base) {
    Type *type = base;
    while (consume(TK_RESERVED, "[")) {
        Token *tok = consume(TK_NUM, NULL);
        int array_size = tok ? tok->val : -1;  // tentatively, array size is -1 when omitted
        expect(TK_RESERVED, "]");
        type = ary_of(type, array_size);
    }
    return type;
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
        enter_scope();
        while (!consume(TK_RESERVED, "}")) {
            vec_push(node->stmts, stmt());
        }
        leave_scope();
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
        enter_scope();
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
        leave_scope();
        return node;
    } else if (at_typename()) {
        declaration();
        return &null_stmt;
    } else if (consume(TK_RESERVED, ";")) {
        return &null_stmt;
    } else {
        node = expr();
    }
    expect(TK_RESERVED, ";");
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
    }
    return node;
}

/**
 * relational = add (("<=" | ">=" | "<" | ">") add)?
 */
Node *relational() {
    Node *node = add();
    if (consume(TK_RESERVED, "<=")) {
        node = new_node_binop(ND_LE, node, add());
    } else if (consume(TK_RESERVED, ">=")) {
        node = new_node_binop(ND_LE, add(), node);
    } else if (consume(TK_RESERVED, "<")) {
        node = new_node_binop(ND_LT, node, add());
    } else if (consume(TK_RESERVED, ">")) {
        node = new_node_binop(ND_LT, add(), node);
    }
    return node;
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
        LVar *lvar = find_lvar(tok->str);
        if (!lvar) {
            GVar *gvar = find_gvar(tok->str);
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
    Func *fn = map_at(prog->funcs, tok->str);
    if (!fn) {
        error_at(tok->loc, "Undeclared function: '%s'", tok->str);
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

Type *new_ty(TypeKind kind, int size, char *repr) {
    Type *type = calloc(1, sizeof(Type));
    type->kind = kind;
    type->size = size;
    type->str = repr;
    return type;
}

Type *int_ty() { return new_ty(TY_INT, 4, "int"); }

Type *char_ty() { return new_ty(TY_CHAR, 1, "char"); }

Type *ptr_to(Type *dest) {
    char *repr = calloc(256, sizeof(char));
    char *s = (dest->kind == TY_ARRAY || dest->kind == TY_PTR) ? "" : " ";
    sprintf(repr, "%s%s*", dest->str, s);
    Type *type = new_ty(TY_PTR, 8, repr);
    type->ptr_to = dest;
    return type;
}

Type *ary_of(Type *base, int size) {
    char *repr = calloc(256, sizeof(char));
    char *s = (base->kind == TY_ARRAY || base->kind == TY_PTR) ? "" : " ";
    sprintf(repr, "%s%s[%d]", base->str, s, size);
    Type *type = new_ty(TY_ARRAY, base->size * size, repr);
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
