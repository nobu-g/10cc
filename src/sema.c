#include "10cc.h"

Node *walk(Node *node) {

    switch (node->kind) {

    }
    return node;
}


void sema(Program *prog) {
    for (int i = 0; i < prog->fns->len; i++) {
        Func *fn = get_elem_from_vec(prog->fns->vals, i);
        for (int j = 0; j < fn->body->len; j++) {
            Node *node = get_elem_from_vec(fn->body, j);
            node = walk(node);
        }
    }
}
