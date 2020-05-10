#include "9cc.h"


char *user_input;
Token *token;

// returns True if the current token is op
bool consume(char *op) {
    if(token->kind != TK_RESERVED || strlen(op) != token->len ||
       memcmp(token->str, op, token->len) != 0) {
        return false;
    }
    token = token->next;
    return true;
}

bool consume_stmt(TokenKind kind) {
    if(token->kind != kind) {
        return false;
    }
    token = token->next;
    return true;
}

Token *consume_ident() {
    if(token->kind != TK_IDENT) {
        return NULL;
    }
    Token *tok = token;
    token = token->next;
    return tok;
}

void expect(char *op) {
    if(token->kind != TK_RESERVED || strlen(op) != token->len ||
       memcmp(token->str, op, token->len) != 0) {
        error_at(token->str, "'%s'ではありません", op);
    }
    token = token->next;
}

int expect_number() {
    if(token->kind != TK_NUM) {
        error_at(token->str, "数ではありません");
    }
    int val = token->val;
    token = token->next;
    return val;
}

Token *new_token(TokenKind kind, Token *cur, char *str, int len) {
    Token *tok = calloc(1, sizeof(Token));
    tok->kind = kind;
    tok->str = str;
    tok->len = len;
    cur->next = tok;
    return tok;
}

void tokenize() {
    Token head;
    head.next = NULL;
    Token *cur = &head;
    char *p = user_input;

    while(*p) {
        if(isspace(*p)) {
            p++;
            continue;
        }

        if(strncmp(p, "return", 6) == 0 && !is_alnum(p[6])) {
            cur = new_token(TK_RETURN, cur, p, 6);
            p += 6;
            continue;
        }

        if(strncmp(p, "if", 2) == 0 && !is_alnum(p[2])) {
            cur = new_token(TK_IF, cur, p, 2);
            p += 2;
            continue;
        }

        if(strncmp(p, "else", 4) == 0 && !is_alnum(p[4])) {
            cur = new_token(TK_ELSE, cur, p, 4);
            p += 4;
            continue;
        }

        if(strncmp(p, "while", 5) == 0 && !is_alnum(p[5])) {
            cur = new_token(TK_WHILE, cur, p, 5);
            p += 5;
            continue;
        }

        if(strncmp(p, "for", 3) == 0 && !is_alnum(p[3])) {
            cur = new_token(TK_FOR, cur, p, 3);
            p += 3;
            continue;
        }

        if(isalpha(*p) || *p == '_') {
            int len = 1;
            while(is_alnum(p[len])) {
                len++;
            }
            cur = new_token(TK_IDENT, cur, p, len);
            p += len;
            continue;
        }

        if(startswith(p, "==") || startswith(p, "!=") || startswith(p, "<=") ||
           startswith(p, ">=")) {
            cur = new_token(TK_RESERVED, cur, p, 2);
            p += 2;
            continue;
        }

        if(strchr("+-*/()<>=;{}", *p)) {
            cur = new_token(TK_RESERVED, cur, p, 1);
            p++;
            continue;
        }

        if(isdigit(*p)) {
            char *tmp = p;
            int val = strtol(p, &p, 10);
            cur = new_token(TK_NUM, cur, p, p - tmp);
            cur->val = val;
            continue;
        }
        error_at(p, "トークナイズできません");
    }

    new_token(TK_EOF, cur, p, 0);
    token = head.next;
}
