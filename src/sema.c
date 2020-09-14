#include "10cc.h"

// Semantics analyzer. This pass plays a few important roles as shown below:
//
// - Add types to nodes. For example, a tree that represents "1+2" is
//   typed as INT because the result type of an addition of two
//   integers is integer.
//
// - Insert nodes to make array-to-pointer conversion explicitly.
//   Recall that, in C, "array of T" is automatically converted to
//   "pointer to T" in most contexts.
//
// - Insert nodes for implicit cast so that they are explicitly
//   represented in AST.
//
// - Scales operands for pointer arithmetic. E.g. ptr+1 becomes ptr+4
//   for integer and becomes ptr+8 for pointer.
//
// - Reject bad assignments, such as `1=2+3`.

Node *new_node_num(int val);
Type *ptr_to(Type *base);
Type *int_ty();


void check_integer(Node *node) {
    int ty = node->ty->ty;
    assert(ty == INT || ty == CHAR, "Not referable");
}

void check_referable(Node *node) {
    NodeKind kind = node->kind;
    assert(kind == ND_LVAR || kind == ND_GVAR || kind == ND_DEREF, "Not an integer");
}

Node *scale_ptr(int op, Node *base, Type *ty) {
    Node *node = calloc(1, sizeof(Node));
    node->kind = op;
    node->lhs = base;
    node->rhs = new_node_num(ty->ptr_to->size);
    return node;
}

bool same_type(Type *x, Type *y) {
    if (x->ty != y->ty) {
        return false;
    }

    switch(x->ty) {
    case PTR:
        return same_type(x->ptr_to, y->ptr_to);
    case ARRAY:
        return x->array_size == y->array_size && same_type(x->ptr_to, y->ptr_to);
    default:
        return true;
    }
}

// TODO: decay 実装？
Node *walk(Node *node) {
    switch (node->kind) {
        case ND_NUM:
            return node;
        case ND_LVAR:
        case ND_GVAR:
            return node;
        case ND_IF:
            node->cond = walk(node->cond);
            node->then = walk(node->then);
            if (node->els) {
                node->els = walk(node->els);
            }
            return node;
        case ND_FOR:
            if (node->init) {
                node->init = walk(node->init);
            }
            if (node->cond) {
                node->cond = walk(node->cond);
            }
            if (node->upd) {
                node->upd = walk(node->upd);
            }
            node->then = walk(node->then);
            return node;
        case ND_WHILE:
            node->cond = walk(node->cond);
            node->then = walk(node->then);
            return node;
        case ND_ADD:
            node->lhs = walk(node->lhs);
            node->rhs = walk(node->rhs);
            if (node->rhs->ty->ty == PTR) {
                Node *tmp = node->lhs;
                node->lhs = node->rhs;
                node->rhs = tmp;
            }
            check_integer(node->rhs);

            if (node->lhs->ty->ty == PTR) {
                node->rhs = scale_ptr(ND_MUL, node->rhs, node->lhs->ty);
                node->ty = node->lhs->ty;
            } else {
                node->ty = int_ty();  // TODO: lhs と rhs のうち小さい方の ty にする
            }
            return node;
        case ND_SUB:
            node->lhs = walk(node->lhs);
            node->rhs = walk(node->rhs);

            Type *lty = node->lhs->ty;
            Type *rty = node->rhs->ty;

            if (lty->ty == PTR) {
                if (rty->ty == PTR) {
                    assert(same_type(lty, rty), "Incompatible pointer");
                    node = scale_ptr(ND_DIV, node, lty);
                } else {
                    node = scale_ptr(ND_MUL, node->rhs, lty);
                }
                node->ty = lty;
            } else {
                if (rty->ty == PTR) {
                    error("Invalid operands: %d and %d", lty, rty);
                }
                node->ty = int_ty();  // TODO
            }
            return node;
        case ND_ASSIGN:
            node->lhs = walk(node->lhs);
            check_referable(node->lhs);
            node->rhs = walk(node->rhs);
            node->ty = node->lhs->ty;  // TODO: lhs と rhs のうち小さい方の ty にする
            return node;
        case ND_MUL:
        case ND_DIV:
        case ND_EQ:
        case ND_NE:
        case ND_LE:
        case ND_LT:
            node->lhs = walk(node->lhs);
            node->rhs = walk(node->rhs);
            check_integer(node->lhs);
            check_integer(node->rhs);
            node->ty = int_ty();
            return node;
        case ND_ADDR:
            node->lhs = walk(node->lhs);
            check_referable(node->lhs);
            node->ty = ptr_to(node->lhs->ty);
            return node;
        case ND_DEREF:
            node->lhs = walk(node->lhs);
            assert(node->lhs->ty->ty == PTR, "Operand must be a pointer");
            node->ty = node->lhs->ty->ptr_to;
            return node;
        case ND_RETURN:
            node->lhs = walk(node->lhs);
            return node;
        case ND_FUNC_CALL:
            for (int i = 0; i < node->args->len; i++) {
                Node *arg = vec_get(node->args, i);
                arg = walk(arg);
            }
            return node;
        case ND_BLOCK:
            for (int i = 0; i < node->stmts->len; i++) {
                Node *stmt = vec_get(node->args, i);
                stmt = walk(stmt);
            }
            return node;
        default:
            error("Unknown node kind: %d", node->kind);
    }
}

void sema(Program *prog) {
    for (int i = 0; i < prog->fns->len; i++) {
        Func *fn = vec_get(prog->fns->vals, i);
        for (int j = 0; j < fn->body->len; j++) {
            Node *node = vec_get(fn->body, j);
            node = walk(node);
        }
    }
}
