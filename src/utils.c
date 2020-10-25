#include "10cc.h"

void debug(char *fmt, ...) {
    va_list ap;
    va_start(ap, fmt);
    vfprintf(stderr, fmt, ap);
}
// Takes a printf-style format string and returns a formatted string.
char *format(char *fmt, ...) {
    size_t size = 2048;
    char *buff = calloc(size, sizeof(char));
    va_list ap;
    va_start(ap, fmt);
    vsnprintf(buff, sizeof(char) * size, fmt, ap);
    va_end(ap);
    return buff;
}

void error(char *fmt, ...) {
    va_list ap;
    va_start(ap, fmt);
    vfprintf(stderr, fmt, ap);
    fprintf(stderr, "\n");
    exit(1);
}

void error_at(char *loc, char *fmt, ...) {
    // find the start/end positions of the line
    char *start = loc;
    while (user_input < start && start[-1] != '\n') {
        start--;
    }
    char *end = loc;
    while (*end != '\n') {
        end++;
    }

    // investigate the line number
    int line_num = 1;
    for (char *p = user_input; p < start; p++) {
        if (*p == '\n') {
            line_num++;
        }
    }

    // report the line number with the file name
    int indent = fprintf(stderr, "%s:%d: ", filename, line_num);
    fprintf(stderr, "%.*s\n", (int)(end - start), start);

    // show the error message with a pointer "^"
    int pos = loc - start + indent;
    fprintf(stderr, "%*s", pos, "");
    fprintf(stderr, "^ ");
    va_list ap;
    va_start(ap, fmt);
    vfprintf(stderr, fmt, ap);
    fprintf(stderr, "\n");
    exit(1);
}

void assert(bool cond, char *fmt, ...) {
    va_list ap;
    va_start(ap, fmt);
    if (!cond) {
        vfprintf(stderr, fmt, ap);
        fprintf(stderr, "\n");
        exit(1);
    }
}

void draw_node_tree(Node *node, int depth, char *prefix) {
    if (node != NULL) {
        for (int i = 0; i < depth; i++) {
            fprintf(stderr, "  ");
        }
        if (strlen(prefix)) {
            fprintf(stderr, "%s: ", prefix);
        }

        switch (node->kind) {
        case ND_ADD:
            fprintf(stderr, "ADD\n");
            draw_node_tree(node->lhs, depth + 1, "lhs");
            draw_node_tree(node->rhs, depth + 1, "rhs");
            break;
        case ND_SUB:
            fprintf(stderr, "SUB\n");
            draw_node_tree(node->lhs, depth + 1, "lhs");
            draw_node_tree(node->rhs, depth + 1, "rhs");
            break;
        case ND_MUL:
            fprintf(stderr, "MUL\n");
            draw_node_tree(node->lhs, depth + 1, "lhs");
            draw_node_tree(node->rhs, depth + 1, "rhs");
            break;
        case ND_DIV:
            fprintf(stderr, "DIV\n");
            draw_node_tree(node->lhs, depth + 1, "lhs");
            draw_node_tree(node->rhs, depth + 1, "rhs");
            break;
        case ND_EQ:
            fprintf(stderr, "EQ\n");
            draw_node_tree(node->lhs, depth + 1, "lhs");
            draw_node_tree(node->rhs, depth + 1, "rhs");
            break;
        case ND_NE:
            fprintf(stderr, "NE\n");
            draw_node_tree(node->lhs, depth + 1, "lhs");
            draw_node_tree(node->rhs, depth + 1, "rhs");
            break;
        case ND_LE:
            fprintf(stderr, "LE\n");
            draw_node_tree(node->lhs, depth + 1, "lhs");
            draw_node_tree(node->rhs, depth + 1, "rhs");
            break;
        case ND_LT:
            fprintf(stderr, "LT\n");
            draw_node_tree(node->lhs, depth + 1, "lhs");
            draw_node_tree(node->rhs, depth + 1, "rhs");
            break;
        case ND_ASSIGN:
            fprintf(stderr, "ASSIGN\n");
            draw_node_tree(node->lhs, depth + 1, "lhs");
            draw_node_tree(node->rhs, depth + 1, "rhs");
            break;
        case ND_RETURN:
            fprintf(stderr, "RETURN\n");
            draw_node_tree(node->lhs, depth + 1, "");
            break;
        case ND_IF:
            fprintf(stderr, "IF\n");
            draw_node_tree(node->cond, depth + 1, "cond");
            draw_node_tree(node->then, depth + 1, "then");
            draw_node_tree(node->els, depth + 1, "else");
            break;
        case ND_WHILE:
            fprintf(stderr, "WHILE\n");
            draw_node_tree(node->cond, depth + 1, "cond");
            draw_node_tree(node->then, depth + 1, "then");
            break;
        case ND_FOR:
            fprintf(stderr, "FOR\n");
            draw_node_tree(node->init, depth + 1, "init");
            draw_node_tree(node->cond, depth + 1, "cond");
            draw_node_tree(node->upd, depth + 1, "update");
            draw_node_tree(node->then, depth + 1, "then");
            break;
        case ND_BLOCK:
            fprintf(stderr, "BLOCK\n");
            for (int i = 0; i < node->stmts->len; i++) {
                draw_node_tree(vec_get(node->stmts, i), depth + 1, "");
            }
            break;
        case ND_STMT_EXPR:
            fprintf(stderr, "STMT_EXPR\n");
            for (int i = 0; i < node->stmts->len; i++) {
                draw_node_tree(vec_get(node->stmts, i), depth + 1, "");
            }
            break;
        case ND_EXPR_STMT:
            fprintf(stderr, "EXPR_STMT\n");
            draw_node_tree(node->lhs, depth + 1, "");
            break;
        case ND_FUNC_CALL:
            fprintf(stderr, "FUNC_CALL(%s)\n", node->func->name);
            for (int i = 0; i < node->args->len; i++) {
                char prefix[16] = {'\0'};
                sprintf(prefix, "arg%d", i);
                draw_node_tree(vec_get(node->args, i), depth + 1, prefix);
            }
            break;
        case ND_VARREF:
            fprintf(stderr, "VARREF(type: %s, name: %s)\n", node->var->type->str, node->var->name);
            break;
        case ND_NUM:
            fprintf(stderr, "NUM(%d)\n", node->val);
            break;
        case ND_STR:
            fprintf(stderr, "STR(\"%s\")\n", node->strl->str);
            break;
        case ND_ADDR:
            fprintf(stderr, "ADDR\n");
            draw_node_tree(node->lhs, depth + 1, "");
            break;
        case ND_DEREF:
            fprintf(stderr, "DEREF\n");
            draw_node_tree(node->lhs, depth + 1, "");
            break;
        case ND_SIZEOF:
            fprintf(stderr, "SIZEOF\n");
            draw_node_tree(node->lhs, depth + 1, "");
            break;
        case ND_NULL:
            fprintf(stderr, "NULL\n");
            break;
        default:
            error("Unknown node kind: %d", node->kind);
        }
    }
}

void draw_ast(Program *prog) {
    for (int i = 0; i < prog->funcs->len; i++) {
        Func *fn = vec_get(prog->funcs->vals, i);
        fprintf(stderr, "%s(", fn->name);
        for (int j = 0; j < fn->args->len; j++) {
            Var *arg = vec_get(fn->args, j);
            if (j > 0) {
                fprintf(stderr, ", ");
            }
            fprintf(stderr, "%s %s", arg->type->str, arg->name);
        }
        fprintf(stderr, ")\n");
        draw_node_tree(fn->body, 1, "");
        fprintf(stderr, "\n");
    }
}
