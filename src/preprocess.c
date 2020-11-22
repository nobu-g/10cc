#include "10cc.h"

Token *copy_token(Token *tok) {
    Token *t = calloc(1, sizeof(Token));
    *t = *tok;
    t->next = NULL;
    return t;
}

Token *append(Token *tok1, Token *tok2) {
    if (!tok1 || tok1->kind == TK_EOF) return tok2;

    Token head = {};
    Token *cur = &head;

    // copy tok1 to cur
    for (; tok1 && tok1->kind != TK_EOF; tok1 = tok1->next) {
        cur = cur->next = copy_token(tok1);
    }
    cur->next = tok2;
    return head.next;
}

Token *preprocess(Token *tok) {
    Token head = {};
    Token *cur = &head;
    while (tok->kind != TK_EOF) {
        if (!(tok->is_bol && equal(tok, "#"))) {
            cur = cur->next = tok;
            tok = tok->next;
            continue;
        }

        tok = tok->next;

        if (equal(tok, "include")) {
            tok = tok->next;
            if (tok->kind != TK_STR) {
                error_at(tok->loc, "filename expected");
            }
            // read file
            File *file = read_file(format("%s/%s", dirname(current_file->name), tok->str));
            debug(file->name);
            // append(cur, file_content);
        }

        if (tok->is_bol) {
            // do nothing for null directive
            continue;
        }

        error_at(tok->loc, "invalid preprocessor directive", tok->str);
    }
    return head.next;
}
