#include "10cc.h"

Func *f;
Vector *code;

Node *new_node(NodeKind kind, Node *lhs, Node *rhs);
Node *new_node_num(int val);
void error_at(char *loc, char *fmt, ...);
LVar *find_lvar(Token *tok);

bool at_eof();

/**
 * program = func*
 */
void program() {
    code = create_vector();
    while(!at_eof()) {
        push(code, func());
    }
}

/**
 * func = stmt*
 */
Func *func() {
    f = calloc(1, sizeof(Func));
    Token *tok = consume(TK_INT, NULL);
    if (tok) {
        f->ret_type = calloc(1, sizeof(Type));
        f->ret_type->ty = INT;
        f->ret_type->size = 4;
        for (;;) {
            if(consume(TK_RESERVED, "*")) {
                Type *type_ = calloc(1, sizeof(Type));
                type_->ty = PTR;
                type_->size = 8;
                type_->ptr_to = f->ret_type;
                f->ret_type = type_;
            } else {
                break;
            }
        }
        tok = consume(TK_IDENT, NULL);
        if(!tok) {
            error("関数名ではありません");
        }
    } else {
        error("有効な型ではありません");
    }
    f->name = tok->str;
    f->lvars = create_map();
    f->args = create_vector();
    expect("(");
    for(;;) {
        Token *tok = consume(TK_INT, NULL);
        if(tok) {
            Node *node = calloc(1, sizeof(Node));
            node->kind = ND_LVAR;
            node->ty = calloc(1, sizeof(Type));
            node->ty->ty = INT;
            node->ty->size = 4;
            for(;;) {
                if(consume(TK_RESERVED, "*")) {
                    Type *type_ = calloc(1, sizeof(Type));
                    type_->ty = PTR;
                    type_->size = 8;
                    type_->ptr_to = node->ty;
                    node->ty = type_;
                } else {
                    break;
                }
            }
            tok = consume(TK_IDENT, NULL);
            if(!tok) {
                error("変数名ではありません");
            }
            LVar *lvar = get_elem_from_map(f->lvars, tok->str);
            if(lvar) {
                error("変数 %s はすでに宣言されています", tok->str);
            }
            lvar = calloc(1, sizeof(LVar));
            lvar->offset = (f->lvars->len + 1) * 8;
            node->offset = lvar->offset;
            push(f->args, node);
            add_elem_to_map(f->lvars, tok->str, lvar);
            if(!consume(TK_RESERVED, ",")) {
                break;
            }
        } else {
            break;
        }
    }
    expect(")");
    expect("{");
    f->body = create_vector();
    while(!consume(TK_RESERVED, "}")) {
        push(f->body, stmt());
    }
    return f;
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
    if(consume(TK_RESERVED, "{")) {
        node = calloc(1, sizeof(Node));
        node->kind = ND_BLOCK;
        node->stmts = create_vector();
        while(!consume(TK_RESERVED, "}")) {
            push(node->stmts, (void *)stmt());
        }
        return node;
    } else if(consume(TK_RETURN, NULL)) {
        node = calloc(1, sizeof(Node));
        node->kind = ND_RETURN;
        node->lhs = expr();
    } else if(consume(TK_IF, NULL)) {
        node = calloc(1, sizeof(Node));
        node->kind = ND_IF;
        expect("(");
        node->cond = expr();
        expect(")");
        node->then = stmt();
        if(consume(TK_ELSE, NULL)) {
            node->els = stmt();
        }
        return node;
    } else if(consume(TK_WHILE, NULL)) {
        node = calloc(1, sizeof(Node));
        node->kind = ND_WHILE;
        expect("(");
        node->cond = expr();
        expect(")");
        node->then = stmt();
        return node;
    } else if(consume(TK_FOR, NULL)) {
        node = calloc(1, sizeof(Node));
        node->kind = ND_FOR;
        expect("(");
        if(!consume(TK_RESERVED, ";")) {
            node->init = expr();
            expect(";");
        }
        if(!consume(TK_RESERVED, ";")) {
            node->cond = expr();
            expect(";");
        }
        if(!consume(TK_RESERVED, ";")) {
            node->upd = expr();
        }
        expect(")");
        node->then = stmt();
        return node;
    } else {
        node = expr();
    }
    if(!consume(TK_RESERVED, ";")) {
        error_at(token->loc, "';'ではないトークンです");
    }
    return node;
}

Node *expr() { return assign(); }

/**
 * assign = equality ("=" assign)?
 */
Node *assign() {
    Node *node = equality();
    if(consume(TK_RESERVED, "=")) {
        node = new_node(ND_ASSIGN, node, assign());
    }
    return node;
}

/**
 * add = mul ("+" unary | "-" unary)*
 */
Node *equality() {
    Node *node = relational();
    if(consume(TK_RESERVED, "==")) {
        node = new_node(ND_EQ, node, relational());
    } else if(consume(TK_RESERVED, "!=")) {
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
    if(consume(TK_RESERVED, "<=")) {
        return new_node(ND_LE, node, add());
    } else if(consume(TK_RESERVED, ">=")) {
        return new_node(ND_LE, add(), node);
    } else if(consume(TK_RESERVED, "<")) {
        return new_node(ND_LT, node, add());
    } else if(consume(TK_RESERVED, ">")) {
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
    for(;;) {
        if(consume(TK_RESERVED, "+")) {
            node = new_node(ND_ADD, node, mul());
        } else if(consume(TK_RESERVED, "-")) {
            node = new_node(ND_SUB, node, mul());
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
    for(;;) {
        if(consume(TK_RESERVED, "*")) {
            node = new_node(ND_MUL, node, unary());
        } else if(consume(TK_RESERVED, "/")) {
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
    if(consume(TK_RESERVED, "+")) {
        return primary();
    } else if(consume(TK_RESERVED, "-")) {
        return new_node(ND_SUB, new_node_num(0), primary());
    } else if(consume(TK_RESERVED, "&")) {
        return new_node(ND_ADDR, NULL, unary());
    } else if(consume(TK_RESERVED, "*")) {
        return new_node(ND_DEREF, NULL, unary());
    } else if(consume(TK_SIZEOF, NULL)) {
        // ここで unary を作って型をチェック -> int にする
    } else {
        return primary();
    }
}

/**
 * primary = num              // 即値
 *         | ident ("(" ")")  // 変数参照 or 関数呼び出し
 *         | "int" ident      // 変数定義
 *         | "(" expr ")"     // 括弧
 */
Node *primary() {
    // 変数宣言
    Token *tok = consume(TK_INT, NULL);
    if(tok) {
        Node *node = calloc(1, sizeof(Node));
        node->kind = ND_LVAR;
        node->ty = calloc(1, sizeof(Type));
        node->ty->ty = INT;
        node->ty->size = 4;
        for(;;) {
            if(consume(TK_RESERVED, "*")) {
                Type *type_ = calloc(1, sizeof(Type));
                type_->ty = PTR;
                type_->size = 8;
                type_->ptr_to = node->ty;
                node->ty = type_;
            } else {
                break;
            }
        }
        tok = consume(TK_IDENT, NULL);
        if(!tok) {
            error("変数名ではありません");
        }
        LVar *lvar = get_elem_from_map(f->lvars, tok->str);
        if(lvar) {
            error("変数 %s はすでに宣言されています", tok->str);
        }
        lvar = calloc(1, sizeof(LVar));
        lvar->offset = (f->lvars->len + 1) * 8;
        node->offset = lvar->offset;
        add_elem_to_map(f->lvars, tok->str, lvar);
        return node;
    }

    // 変数参照 or 関数呼び出し
    tok = consume(TK_IDENT, NULL);
    if (tok) {
        Node *node = calloc(1, sizeof(Node));
        // 関数呼び出し
        if(consume(TK_RESERVED, "(")) {
            node->kind = ND_FUNC_CALL;
            node->name = tok->str;
            node->args = create_vector();
            for(;;) {
                if(consume(TK_RESERVED, ")")) {
                    break;
                }
                Node *arg = expr();
                push(node->args, arg);
                consume(TK_RESERVED, ",");
            }
        // 変数参照
        } else {
            node->kind = ND_LVAR;
            LVar *lvar = get_elem_from_map(f->lvars, tok->str);
            if (lvar) {
                node->offset = lvar->offset;
            } else {
                error("%sは未定義です", tok->str);
            }
        }
        return node;
    }
    if(consume(TK_RESERVED, "(")) {
        Node *node = expr();
        expect(")");
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

bool at_eof() { return token->kind == TK_EOF; }
