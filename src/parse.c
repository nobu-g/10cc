#include "10cc.h"

Program *prog;               // The program
Scope *scope;                // Current scope
Node null_stmt = {ND_NULL};  // NOP node
int str_label_cnt = 1;

void top_level();
Var *param_declaration();
void declaration();
Type *read_array(Type *base);
Func *new_func(Type *ret_type, char *name, Vector *args);
Var *new_var(Type *type, char *name, bool is_local);

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

Func *find_func(char *name);
Var *find_var(char *name);

Node *new_node(NodeKind kind);
Node *new_node_uniop(NodeKind kind, Node *lhs);
Node *new_node_binop(NodeKind kind, Node *lhs, Node *rhs);
Node *new_node_func_call(Func *func, Vector *args);
Node *new_node_varref(Var *var);
Node *new_node_string(char *str);
Node *new_node_num(int val);

Type *read_type();

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
    prog->strls = map_create();
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

        Func *func = new_func(type, tok->str, args);

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
        new_var(type, tok->str, false);
    }
}

/*
 * param_declaration = T IDENT ("[" NUM? "]")*
 */
Var *param_declaration() {
    Type *type = read_type();
    Token *tok = expect(TK_IDENT, NULL);
    type = read_array(type);
    return new_var(type, tok->str, true);
}

/*
 * declaration = T IDENT ("[" NUM? "]")* ";"
 */
void declaration() {
    Type *type = read_type();
    Token *tok = expect(TK_IDENT, NULL);
    type = read_array(type);
    new_var(type, tok->str, true);
    expect(TK_RESERVED, ";");
}

Type *read_array(Type *base) {
    Vector *sizes = vec_create();
    while (consume(TK_RESERVED, "[")) {
        Token *tok = consume(TK_NUM, NULL);
        vec_pushi(sizes, tok ? tok->val : -1);  // tentatively, array size is -1 when omitted
        expect(TK_RESERVED, "]");
    }
    Type *type = base;
    for (int i = sizes->len - 1; i >= 0; i--) {
        type = ary_of(type, vec_geti(sizes, i));
    }
    return type;
}

Func *new_func(Type *ret_type, char *name, Vector *args) {
    Func *fn = map_at(prog->funcs, name);
    if (fn) {
        bool is_compatible = same_type(ret_type, fn->ret_type) && (args->len == fn->args->len);
        if (is_compatible) {
            for (int i = 0; i < args->len; i++) {
                Var *arg = vec_get(args, i);
                Var *fn_arg = vec_get(fn->args, i);
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

Var *new_var(Type *type, char *name, bool is_local) {
    Map *vars = is_local ? scope->lvars : prog->gvars;
    Var *var = map_at(vars, name);
    if (var) {
        if (!same_type(type, var->type)) {
            error_at(token->loc, "Redefinition of '%s' with a different type: '%s' vs '%s'", name, type->str,
                     var->type->str);
        }
    } else {
        var = calloc(1, sizeof(Var));
        var->is_local = is_local;
        var->name = name;
        var->type = type;
        map_insert(vars, name, var);
    }
    return var;
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
 *         | STR                                // string literal
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
        if (consume(TK_RESERVED, "(")) {
            // function call
            Func *fn = find_func(tok->str);
            Vector *args = vec_create();
            while (!consume(TK_RESERVED, ")")) {
                if (args->len > 0) {
                    expect(TK_RESERVED, ",");
                }
                vec_push(args, expr());
            }
            return new_node_func_call(fn, args);
        }

        // variable reference
        Var *var = find_var(tok->str);
        return new_node_varref(var);
    }

    // string literal
    if (consume(TK_RESERVED, "\"")) {
        Token* tok = consume(TK_STR, NULL);
        expect(TK_RESERVED, "\"");
        return new_node_string(tok->str);
    }

    // immediate value
    return new_node_num(expect(TK_NUM, NULL)->val);
}

Func *find_func(char *name) {
    Func *fn = map_at(prog->funcs, name);
    if (!fn) {
        error_at(token->loc, "Undeclared function: '%s'", name);
    }
    return fn;
}

Var *find_var(char *name) {
    Var *var = NULL;
    Scope *sc = scope;
    while (sc) {
        var = map_at(sc->lvars, name);
        if (var) {
            break;
        }
        sc = sc->parent;
    }
    if (!var) {
        var = map_at(prog->gvars, name);
    }
    if (!var) {
        error_at(token->loc, "Undefined variable: '%s'", name);
    }
    return var;
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

Node *new_node_func_call(Func *func, Vector *args) {
    Node *node = new_node(ND_FUNC_CALL);
    node->func = func;
    node->args = args;
    return node;
}

Node *new_node_varref(Var *var) {
    Node *node = new_node(ND_VARREF);
    node->var = var;
    return node;
}

Node *new_node_string(char *str) {
    StrLiteral *strl = calloc(1, sizeof(StrLiteral));
    strl->label = format(".L.str%d", str_label_cnt++);
    strl->str = str;
    map_insert(prog->strls, str, strl);
    Node *node = new_node(ND_STR);
    node->type = ary_of(type_char, strlen(str) + 1);
    node->strl = strl;
    return node;
}

Node *new_node_num(int val) {
    Node *node = new_node(ND_NUM);
    node->val = val;
    node->type = type_int;
    return node;
}

Type *read_type() {
    Type *type;
    if (consume(TK_RESERVED, "int")) {
        type = type_int;
    } else if (consume(TK_RESERVED, "char")) {
        type = type_char;
    } else {
        error_at(token->loc, "Invalid type");
    }
    while (consume(TK_RESERVED, "*")) {
        type = ptr_to(type);
    }
    return type;
}
