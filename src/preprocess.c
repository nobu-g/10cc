#include "10cc.h"

bool is_hash(Token *tok) {
    return strcmp(tok->str, "#") == 0;
}

Token *preprocess(Token *tok) {
    Token head = {};
    Token *cur = &head;
    while (tok->kind != TK_EOF) {
        if (!(tok->is_bol && is_hash(tok))) {
            cur = cur->next = tok;
            tok = tok->next;
            continue;
        }

        tok = tok->next;

        if (tok->is_bol) {
            // do nothing for null directive
            continue;
        }

        error_at(tok->loc, "invalid preprocessor directive");
    }
    return head.next;
}
