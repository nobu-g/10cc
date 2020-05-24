#include "10cc.h"


Node *code[100];
Map *locals;  // local varables

Node *new_node(NodeKind kind, Node *lhs, Node *rhs);
Node *new_node_num(int val);
void error_at(char *loc, char *fmt, ...);
LVar *find_lvar(Token *tok);

bool at_eof();

void program() {
    locals = create_map();
    int i = 0;
    while(!at_eof()) {
        code[i++] = stmt();
    }
    code[i] = NULL;
}

Node *stmt() {
    Node *node;
    if (consume(TK_RESERVED, "{")) {
        node = calloc(1, sizeof(Node));
        node->kind = ND_BLOCK;
        node->stmts = create_vector();
        while(!consume(TK_RESERVED, "}")) {
            push(node->stmts, (void *) stmt());
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

Node *expr() { return assign(); }

Node *assign() {
    Node *node = equality();
    if(consume(TK_RESERVED, "=")) {
        node = new_node(ND_ASSIGN, node, assign());
    }
    return node;
}

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

Node *unary() {
    if(consume(TK_RESERVED, "+")) {
        return primary();
    } else if(consume(TK_RESERVED, "-")) {
        return new_node(ND_SUB, new_node_num(0), primary());
    } else {
        return primary();
    }
}

Node *primary() {
    Token *tok = consume(TK_IDENT, NULL);
    if (tok) {
        Node *node = calloc(1, sizeof(Node));
        if (consume(TK_RESERVED, "(")) {
            node->kind = ND_FUNC_CALL;
            node->name = tok->str;
            node->args = create_vector();
            for(;;) {
                if (consume(TK_RESERVED, ")")) {
                    break;
                }
                Node *arg = expr();
                push(node->args, arg);
                consume(TK_RESERVED, ",");
            }
        } else {
            node->kind = ND_LVAR;
            LVar *lvar = get_elem_from_map(locals, tok->str);
            if (lvar) {
                node->offset = lvar->offset;
            } else {
                lvar = calloc(1, sizeof(LVar));
                lvar->offset = (locals->len + 1) * 8;
                node->offset = lvar->offset;
                add_elem_to_map(locals, tok->str, lvar);
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
