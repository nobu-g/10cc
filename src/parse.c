#include "10cc.h"

Program *prog;
Scope *scope;                // Current scope
Node null_stmt = {ND_NULL};  // NOP node
int str_label_cnt = 1;

typedef struct {
    Vector *arr;  // Vector<InitValue *>
    Node *scalar;
} InitValue;

void top_level();
Var *param_declaration();
Node *declaration();
Type *read_array(Type *base);
Func *new_func(Type *ret_type, char *name, Vector *args);
Var *new_var(Type *type, char *name, bool is_local);

Node *stmt();
Node *expr_stmt();
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
Node *new_node_block(Vector *stmts);
Node *new_node_func_call(Func *func, Vector *args);
Node *new_node_varref(Var *var);
Node *new_node_string(char *str);
Node *new_node_num(int val);

Type *read_type();

Scope *new_scope();
void enter_scope();
void leave_scope();

/**
 * program = top_level* EOF
 */
Program *parse() {
    prog = calloc(1, sizeof(Program));
    prog->funcs = map_create();
    prog->gvars = map_create();
    prog->strls = vec_create();
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
        Vector *stmts = vec_create();
        while (!consume(TK_RESERVED, "}")) {
            vec_push(stmts, stmt());
        }
        func->body = new_node_block(stmts);
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

Node *assign_init(Node *lhs, Type *ltype, InitValue *rhs) {
    Node *init;
    if (ltype->kind == TY_ARRAY) {
        Vector *stmts = vec_create();
        for (int i = 0; i < rhs->arr->len; i++) {
            Node* add = new_node_binop(ND_ADD, lhs, new_node_num(i));
            Node *deref = new_node_uniop(ND_DEREF, add);
            vec_push(stmts, assign_init(deref, ltype->ptr_to, vec_get(rhs->arr, i)));
        }
        if (ltype->array_size == -1) {
            ltype->array_size = rhs->arr->len;
            ltype->size = ltype->ptr_to->size * ltype->array_size;
        }
        for (int i = rhs->arr->len; i < ltype->array_size; i++) {
            Node* add = new_node_binop(ND_ADD, lhs, new_node_num(i));
            Node *deref = new_node_uniop(ND_DEREF, add);
            InitValue *val = calloc(1, sizeof(InitValue));
            val->scalar = new_node_num(0);
            vec_push(stmts, assign_init(deref, ltype->ptr_to, val));
        }
        init = new_node_block(stmts);
    } else {
        if (!rhs->scalar) {
            // int a = {3};
            if (rhs->arr->len == 0) {
                error_at(token->loc, "Scalar initializer cannot be empty");
            }
            if (rhs->arr->len > 1) {
                error_at(token->loc, "Excess elements in scalar initializer");
            }
            rhs->scalar = ((InitValue *)vec_get(rhs->arr, 0))->scalar;
        }
        Node *assign = new_node_binop(ND_ASSIGN, lhs, rhs->scalar);
        init = new_node_uniop(ND_EXPR_STMT, assign);
    }
    return init;
}

InitValue *read_init() {
    InitValue *val = calloc(1, sizeof(InitValue));
    if (consume(TK_RESERVED, "{")) {
        Vector *inits = vec_create();
        while (!consume(TK_RESERVED, "}")) {
            if (inits->len > 0) {
                expect(TK_RESERVED, ",");
            }
            vec_push(inits, read_init());
        }
        val->arr = inits;
    } else if (consume(TK_RESERVED, "\"")) {
        Token* tok = consume(TK_STR, NULL);
        expect(TK_RESERVED, "\"");
        Vector *inits = vec_create();
        for (int i = 0;; i++) {
            InitValue *ch = calloc(1, sizeof(InitValue));
            ch->scalar = new_node_num(tok->str[i]);  // FIXME: character literal
            vec_push(inits, ch);
            if (!tok->str[i]) {
                break;
            }
        }
        val->arr = inits;
    } else {
        val->scalar = expr();
    }
    return val;
}

/*
 * declaration = T IDENT ("[" NUM? "]")* ("=" init)? ";"
 */
Node *declaration() {
    Type *type = read_type();
    Token *tok = expect(TK_IDENT, NULL);
    type = read_array(type);
    Var *var = new_var(type, tok->str, true);

    Node *init = &null_stmt;
    if (consume(TK_RESERVED, "=")) {
        Node *lhs = new_node_varref(var);
        InitValue *rhs = read_init();
        init = assign_init(lhs, type, rhs);
    }
    expect(TK_RESERVED, ";");
    return init;
}

Type *read_array(Type *base) {
    if (!consume(TK_RESERVED, "[")) {
        return base;
    }
    // a[2][3] means 2-length array of 3-length array of int
    Token *tok = consume(TK_NUM, NULL);
    expect(TK_RESERVED, "]");
    base = read_array(base);
    return ary_of(base, tok ? tok->val : -1);  // array size is set to -1 when omitted
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
        } else {
            error_at(token->loc, "Redefinition of '%s'", name);
        }
    }
    var = calloc(1, sizeof(Var));
    var->is_local = is_local;
    var->name = name;
    var->type = type;
    map_insert(vars, name, var);
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
    if (consume(TK_RESERVED, "{")) {
        enter_scope();
        Vector *stmts = vec_create();
        while (!consume(TK_RESERVED, "}")) {
            vec_push(stmts, stmt());
        }
        leave_scope();
        return new_node_block(stmts);
    } else if (consume(TK_RESERVED, "return")) {
        Node *node = new_node_uniop(ND_RETURN, expr());
        expect(TK_RESERVED, ";");
        return node;
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
        enter_scope();
        expect(TK_RESERVED, "(");
        node->init = peek(TK_RESERVED, ";") ? &null_stmt : expr_stmt();
        expect(TK_RESERVED, ";");
        node->cond = peek(TK_RESERVED, ";") ? new_node_num(1) : expr();
        expect(TK_RESERVED, ";");
        node->upd = peek(TK_RESERVED, ")") ? &null_stmt : expr_stmt();
        expect(TK_RESERVED, ")");
        node->then = stmt();
        leave_scope();
        return node;
    } else if (at_typename()) {
        return declaration();
    } else if (consume(TK_RESERVED, ";")) {
        return &null_stmt;
    } else {
        Node *node = expr_stmt();
        expect(TK_RESERVED, ";");
        return node;
    }
}

/**
 * expr_stmt = expr ";"
 * Expression statement is an expression that does not return any value.
 * If the value of an expression is dismissed, the expression must be a statement expression.
 */
Node *expr_stmt() {
    Node *node = new_node_uniop(ND_EXPR_STMT, expr());
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
 *       | ("++" | "--") unary
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
    } else if (consume(TK_RESERVED, "++")) {
        // `++x` is compiled as `x = x + 1`
        Node *node = unary();
        return new_node_binop(ND_ASSIGN, node, new_node_binop(ND_ADD, node, new_node_num(1)));
    } else if (consume(TK_RESERVED, "--")) {
        // `--x` is compiled as `x = x - 1`
        Node *node = unary();
        return new_node_binop(ND_ASSIGN, node, new_node_binop(ND_SUB, node, new_node_num(1)));
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
 * postfix = primary ("[" expr "]" | "++" | "--" | ".")*
 */
Node *postfix() {
    Node *node = primary();

    while (true) {
        if (consume(TK_RESERVED, "[")) {
            // `x[y]` is compiled as `*(x + y)`
            Node *sbsc = expr();
            expect(TK_RESERVED, "]");
            Node *sum = new_node_binop(ND_ADD, node, sbsc);
            node = new_node_uniop(ND_DEREF, sum);
            continue;
        }
        if (consume(TK_RESERVED, "++")) {
            // `x++` is compiled as `(x = x + 1) - 1`
            Node *assign = new_node_binop(ND_ASSIGN, node, new_node_binop(ND_ADD, node, new_node_num(1)));
            node = new_node_binop(ND_SUB, assign, new_node_num(1));
            continue;
        }
        if (consume(TK_RESERVED, "--")) {
            // `x--` is compiled as `(x = x - 1) + 1`
            Node *assign = new_node_binop(ND_ASSIGN, node, new_node_binop(ND_SUB, node, new_node_num(1)));
            node = new_node_binop(ND_ADD, assign, new_node_num(1));
            continue;
        }
        if (consume(TK_RESERVED, ".")) {
            Token *tok = expect(TK_IDENT, NULL);
            Member *member = map_at(node->var->type->members, tok->str);
            if (!member) {
                error_at(token->loc, "No member named '%s'", tok->str);
            }
            node = new_node_uniop(ND_MEMBER, node);
            node->member = member;
            continue;
        }
        return node;
    }
}

/**
 * primary = "(" expr ")"                       // expression
 *         | "(" "{" stmt+ "}" ")"              // statement expression
 *         | IDENT "(" (expr ("," expr)*)? ")"  // function call
 *         | IDENT                              // variable reference
 *         | STR                                // string literal
 *         | NUM                                // immediate value
 */
Node *primary() {
    // expression
    if (consume(TK_RESERVED, "(")) {
        Node *node;
        if (consume(TK_RESERVED, "{")) {
            enter_scope();
            node = new_node(ND_STMT_EXPR);
            node->stmts = vec_create();
            while (!consume(TK_RESERVED, "}")) {
                vec_push(node->stmts, stmt());
            }
            Node *last = vec_get(node->stmts, node->stmts->len - 1);
            vec_set(node->stmts, node->stmts->len - 1, last->lhs);
            leave_scope();
        } else {
            node = expr();
        }
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

Node *new_node_block(Vector *stmts) {
    Node *node = new_node(ND_BLOCK);
    node->stmts = stmts;
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
    vec_push(prog->strls, strl);
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

Member *new_struct_member(char *name, Type *type) {
    Member *mem = calloc(1, sizeof(Member));
    mem->name = name;
    mem->type = type;
    return mem;
}

Type *read_type() {
    Type *type;
    if (consume(TK_RESERVED, "int")) {
        type = type_int;
    } else if (consume(TK_RESERVED, "char")) {
        type = type_char;
    } else if (consume(TK_RESERVED, "struct")) {
        expect(TK_RESERVED, "{");
        Map *members = map_create();
        int offset = 0;
        while (!consume(TK_RESERVED, "}")) {
            Type *memty = read_type();
            Token *tok = expect(TK_IDENT, NULL);
            memty = read_array(memty);
            Member *mem = new_struct_member(tok->str, memty);
            if (map_at(members, mem->name)) {
                error_at(token->loc, "duplicate member: '%s'", mem->name);
            }
            mem->offset = offset;
            offset += mem->type->size;
            map_insert(members, mem->name, mem);
            expect(TK_RESERVED, ";");
        }
        type = new_type_struct(members);
    } else {
        error_at(token->loc, "Invalid type");
    }
    while (consume(TK_RESERVED, "*")) {
        type = ptr_to(type);
    }
    return type;
}

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
