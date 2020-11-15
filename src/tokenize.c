#include "10cc.h"

Token *token;
bool is_bol;

bool startswith(char *p, char *q);
bool is_alnum(char c);
char *read_reserved(char *p);
Token *new_token(TokenKind kind, Token *cur, char *str, int len);

void tokenize() {
    Token head;
    head.next = NULL;
    Token *cur = &head;
    char *p = user_input;
    is_bol = true;

    while (*p) {

        // skip newline
        if (*p == '\n') {
            p++;
            is_bol = true;
            continue;
        }
        // skip space characters
        if (isspace(*p)) {
            p++;
            continue;
        }

        // skip line comment
        if (strncmp(p, "//", 2) == 0) {
            p += 2;
            while (*p != '\n') {
                p++;
            }
            continue;
        }

        // skip block comment
        if (strncmp(p, "/*", 2) == 0) {
            char *q = strstr(p + 2, "*/");
            if (!q) {
                error_at(p, "unterminated comment");
            }
            p = q + 2;
            continue;
        }

        // read reserved word
        char *kw = read_reserved(p);
        if (kw) {
            int len = strlen(kw);
            cur = new_token(TK_RESERVED, cur, p, len);
            p += len;
            continue;
        }

        // read identifier
        if (isalpha(*p) || *p == '_') {
            int len = 1;
            while (is_alnum(p[len])) {
                len++;
            }
            cur = new_token(TK_IDENT, cur, p, len);
            p += len;
            continue;
        }

        // read number
        if (isdigit(*p)) {
            char *tmp = p;
            int val = strtol(p, &p, 10);
            cur = new_token(TK_NUM, cur, p, p - tmp);
            cur->val = val;
            continue;
        }

        // read string literal
        if (*p == '"') {
            cur = new_token(TK_RESERVED, cur, p, 1);
            p++;
            int len = 0;
            while (p[len] && p[len] != '"') {
                if (p[len] == '\\') {
                    len++;
                }
                len++;
            }
            cur = new_token(TK_STR, cur, p, len);
            p += len;
            cur = new_token(TK_RESERVED, cur, p, 1);
            p++;
            continue;
        }

        error_at(p, "Failed to tokenize user input");
    }

    new_token(TK_EOF, cur, p, 0);
    token = head.next;
}

char *read_reserved(char *p) {
    char *kws[] = {"return", "if", "else", "while", "for", "int", "char", "sizeof", "struct"};
    for (int i = 0; i < sizeof(kws) / sizeof(kws[0]); i++) {
        int len = strlen(kws[i]);
        if (startswith(p, kws[i]) && !is_alnum(p[len])) {
            return kws[i];
        }
    }

    char *multi_ops[] = {"<=", ">=", "==", "!=", "++", "--", "->", "+=", "-=", "*=", "/="};
    for (int i = 0; i < sizeof(multi_ops) / sizeof(multi_ops[0]); i++) {
        if (startswith(p, multi_ops[i])) {
            return multi_ops[i];
        }
    }

    char *single_ops[] = {"+", "-", "*", "/", "(", ")", "<", ">", "=", ";", "{", "}", ",", "[", "]", "&", ".", "#"};
    for (int i = 0; i < sizeof(single_ops) / sizeof(single_ops[0]); i++) {
        if (startswith(p, single_ops[i])) {
            return single_ops[i];
        }
    }
    return NULL;
}

bool startswith(char *p, char *q) { return memcmp(p, q, strlen(q)) == 0; }

bool is_alnum(char c) {
    return ('a' <= c && c <= 'z') || ('A' <= c && c <= 'Z') || ('0' <= c && c <= '9') || (c == '_');
}

Token *new_token(TokenKind kind, Token *cur, char *str, int len) {
    Token *tok = calloc(1, sizeof(Token));

    char *name = calloc(len + 1, sizeof(char));
    strncpy(name, str, len);
    name[len] = '\0';

    tok->kind = kind;
    tok->str = name;
    tok->loc = str;
    tok->is_bol = is_bol;

    cur->next = tok;
    is_bol = false;
    return tok;
}

// Returns the current token if it satisfies given conditions
Token *peek(TokenKind kind, char *str) {
    if (token->kind != kind) {
        return NULL;
    }

    if (str != NULL) {
        if (strcmp(token->str, str) != 0) {
            return NULL;
        }
    }
    return token;
}

Token *consume(TokenKind kind, char *str) {
    Token *ret = peek(kind, str);
    if (ret) {
        token = token->next;
    }
    return ret;
}

Token *expect(TokenKind kind, char *str) {
    if (token->kind != kind || (str && strcmp(token->str, str) != 0)) {
        if (str) {
            error_at(token->loc, "'%s' expected", str);
        } else {
            char *kind_name;
            switch (kind) {
            case TK_RESERVED:
                kind_name = "reserved token";
                break;
            case TK_IDENT:
                kind_name = "identifier";
                break;
            case TK_NUM:
                kind_name = "number";
                break;
            case TK_STR:
                kind_name = "string literal";
                break;
            case TK_EOF:
                kind_name = "EOF";
                break;
            }
            error_at(token->loc, "%s expected", kind_name);
        }
    }

    Token *tok = token;
    token = token->next;
    return tok;
}

bool at_typename() {
    return peek(TK_RESERVED, "int") || peek(TK_RESERVED, "char") || peek(TK_RESERVED, "struct");
}

bool at_eof() {
    return peek(TK_EOF, NULL);
}
