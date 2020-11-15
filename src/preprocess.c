#include "10cc.h"

bool is_hash(Token *tok) {
    return strcmp(tok->str, "#") == 0;
}

Token *preprocess(Token *tok) {
    Token *pre_tok = &(Token){};
    pre_tok->next = tok;
    Token *head = pre_tok;
    while (tok->kind != TK_EOF) {
        if (!(tok->is_bol && is_hash(tok))) {
            pre_tok = tok;
            tok = tok->next;
            continue;
        }

        if (tok->next->is_bol) {
            // do nothing for null directive
            pre_tok->next = tok->next;
            tok = tok->next;
            continue;
        }
    }
    return head->next;
}
