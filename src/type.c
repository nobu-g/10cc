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

Type *type_char = &(Type){TY_CHAR, 1, NULL, 0, "char"};
Type *type_int = &(Type){TY_INT, 4, NULL, 0, "int"};

void check_integer(Node *node) {
    TypeKind kind = node->type->kind;
    assert(kind == TY_INT || kind == TY_CHAR, "Not an integer type: '%s'", node->type->str);
}

void check_referable(Node *node) {
    NodeKind kind = node->kind;
    assert(kind == ND_VARREF || kind == ND_DEREF, "Not a referable type: '%s", node->type->str);
}

Node *scale_ptr(int op, Node *base, Type *type) {
    Node *size = new_node_num(type->ptr_to->size);
    return new_node_binop(op, base, size);
}

Node *ary_to_ptr(Node *base) {
    if (base->type->kind != TY_ARRAY) {
        return base;
    }
    // &(base[0])
    Node *node = new_node_uniop(ND_ADDR, base);
    node->type = ptr_to(base->type->ptr_to);
    return node;
}

Node *do_walk(Node *node, bool decay);

Node *walk(Node *node) { return do_walk(node, true); }

Node *walk_nodecay(Node *node) { return do_walk(node, false); }

Node *do_walk(Node *node, bool decay) {
    assert(node, "Cannot walk on NULL node\n");
    Node *new;
    switch (node->kind) {
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
    case ND_BLOCK:
        for (int i = 0; i < node->stmts->len; i++) {
            Node *stmt = vec_get(node->stmts, i);
            vec_set(node->stmts, i, walk(stmt));
        }
        return node;
    case ND_STMT_EXPR:
        for (int i = 0; i < node->stmts->len; i++) {
            Node *stmt = vec_get(node->stmts, i);
            vec_set(node->stmts, i, walk(stmt));
        }
        Node *last = vec_get(node->stmts, node->stmts->len - 1);
        assert(last->kind != ND_EXPR_STMT, "statement expression returning void is not supported");
        node->type = last->type;
        return node;
    case ND_EXPR_STMT:
        node->lhs = walk(node->lhs);
        return node;
    case ND_ADD:
        new = new_node(node->kind);  // create a new node for non-idempotent node
        new->lhs = walk(node->lhs);
        new->rhs = walk(node->rhs);
        if (new->rhs->type->kind == TY_PTR) {
            Node *tmp = new->lhs;
            new->lhs = new->rhs;
            new->rhs = tmp;
        }
        check_integer(new->rhs);

        if (new->lhs->type->kind == TY_PTR) {
            new->rhs = scale_ptr(ND_MUL, new->rhs, new->lhs->type);
            new->rhs->type = type_int;
            new->type = new->lhs->type;
        } else {
            new->type = type_int;
        }
        return new;
    case ND_SUB:
        new = new_node(node->kind);
        new->lhs = walk(node->lhs);
        new->rhs = walk(node->rhs);

        Type *lty = new->lhs->type;
        Type *rty = new->rhs->type;

        if (lty->kind == TY_PTR) {
            if (rty->kind == TY_PTR) {
                assert(same_type(lty, rty), "Incompatible pointer: '%s' vs '%s'", lty->str, rty->str);
                new = scale_ptr(ND_DIV, new, lty);
                new->type = type_int;  // FIXME: should be long type
            } else {
                new->rhs = scale_ptr(ND_MUL, new->rhs, lty);
                new->type = lty;
            }
        } else {
            assert(rty->kind != TY_PTR, "Invalid operands: '%s' and '%s'", lty->str, rty->str);
            new->type = type_int;
        }
        return new;
    case ND_ASSIGN:
        node->lhs = walk_nodecay(node->lhs);
        check_referable(node->lhs);
        node->rhs = walk(node->rhs);
        node->type = node->lhs->type;
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
        node->type = type_int;
        return node;
    case ND_ADDR:
        node->lhs = walk(node->lhs);
        check_referable(node->lhs);
        node->type = ptr_to(node->lhs->type);
        return node;
    case ND_DEREF:
        new = new_node(node->kind);
        new->lhs = walk(node->lhs);
        assert(new->lhs->type->kind == TY_PTR, "Operand must be a pointer, but got '%s'", new->lhs->type->str);
        new->type = new->lhs->type->ptr_to;
        if (decay) {
            new = ary_to_ptr(new);
        }
        return new;
    case ND_RETURN:
        node->lhs = walk(node->lhs);
        return node;
    case ND_FUNC_CALL:
        for (int i = 0; i < node->args->len; i++) {
            Node *arg = walk(vec_get(node->args, i));
            Type *type_expected = ((Var *)vec_get(node->func->args, i))->type;
            assert(same_type(arg->type, type_expected), "Argument type mismatch: '%s' vs '%s'", arg->type->str,
                   type_expected->str);
            vec_set(node->args, i, arg);
        }
        node->type = node->func->ret_type;
        return node;
    case ND_SIZEOF:
        return new_node_num(walk_nodecay(node->lhs)->type->size);
    case ND_VARREF:
        node->type = node->var->type;
        if (decay) {
            node = ary_to_ptr(node);
        }
        return node;
    case ND_STR:
        if (decay) {
            node = ary_to_ptr(node);
        }
        return node;
    case ND_NUM:
    case ND_NULL:
        return node;
    default:
        error("Unknown node kind: %d", node->kind);
    }
}

void add_type(Program *prog) {
    for (int i = 0; i < prog->funcs->len; i++) {
        Func *fn = vec_get(prog->funcs->vals, i);
        if (fn->body) {
            fn->body = walk(fn->body);
        }
    }
}

Type *new_ty(TypeKind kind, int size, char *repr) {
    Type *type = calloc(1, sizeof(Type));
    type->kind = kind;
    type->size = size;
    type->str = repr;
    return type;
}

Type *ptr_to(Type *dest) {
    char *s = (dest->kind == TY_ARRAY || dest->kind == TY_PTR) ? "" : " ";
    Type *type = new_ty(TY_PTR, 8, format("%s%s*", dest->str, s));
    type->ptr_to = dest;
    return type;
}

Type *ary_of(Type *base, int array_size) {
    char *s = (base->kind == TY_ARRAY || base->kind == TY_PTR) ? "" : " ";
    char *repr;
    if (array_size != -1) {
        repr = format("%s%s[%d]", base->str, s, array_size);
    } else {
        repr = format("%s%s[]", base->str, s);
    }
    Type *type = new_ty(TY_ARRAY, base->size * array_size, repr);
    type->ptr_to = base;
    type->array_size = array_size;
    return type;
}

bool same_type(Type *x, Type *y) {
    if (x->kind != y->kind) {
        return false;
    }

    switch (x->kind) {
    case TY_PTR:
        return same_type(x->ptr_to, y->ptr_to);
    case TY_ARRAY:
        return x->array_size == y->array_size && same_type(x->ptr_to, y->ptr_to);
    default:
        return true;
    }
}
