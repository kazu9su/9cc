#include "9cc.h"

void error(char *fmt, ...);
bool is_alpha(char c);
bool is_alnum(char c);
bool is_block(char c);
bool startswith(char *p, char *q);
char *starts_with_reserved(char *p);
Token *new_token(TokenKind kind, Token *cur, char *str, int len);

Token *tokenize(char *p) {
    Token head;
    head.next = NULL;
    Token *cur = &head;

    while (*p) {
        if (isspace(*p)) {
            p++;
            continue;
        }

        // Keyword or multi-letter punctuators
        char *kw = starts_with_reserved(p);
        if (kw) {
            int len = strlen(kw);
            cur = new_token(TK_RESERVED, cur, p, len);
            p += len;
            continue;
        }

        if (is_alpha(*p)) {
            char *q = p++;
            while (is_alnum(*p))
                p++;
            cur = new_token(TK_IDENT, cur, q, p - q);
            continue;
        }

        if (ispunct(*p) || is_block(*p)) {
            cur = new_token(TK_RESERVED, cur, p++, 1);
            continue;
        }

        if (isdigit(*p)) {
            cur = new_token(TK_NUM, cur, p, 0);
            char *q = p;
            cur->val = strtol(p, &p, 10);
            cur->len = p - q;
            continue;
        }

        error("トークナイズできません");
    }

    return head.next;
}

// 新しいTokenを作成して*curに繋げる
Token *new_token(TokenKind kind, Token *cur, char *str, int len) {
    Token *tok = calloc(1, sizeof(Token));
    tok->kind = kind;
    tok->str = str;
    tok->len = len;
    cur->next = tok;

    return tok;
}

bool is_block(char c) {
    return c == '{' || c == '}';
}

bool is_alpha(char c) {
    return ('a' <= c && c <= 'z') || ('A' <= c && c <= 'Z') || c == '_';
}

bool is_alnum(char c) {
    return is_alpha(c) || ('0' <= c && c <= '9');
}

void error(char *fmt, ...) {
    va_list ap;
    va_start(ap, fmt);
    vfprintf(stderr, fmt, ap);
    fprintf(stderr, "\n");
    exit(1);
}

void verror_at(char *loc, char *fmt, va_list ap) {
    int pos = loc - user_input;
    fprintf(stderr, "%s\n", user_input);
    fprintf(stderr, "%*s", pos, "");
    fprintf(stderr, "^ ");
    fprintf(stderr, fmt, ap);
    fprintf(stderr, "\n");
    exit(1);
}

void error_at(char *loc, char *fmt, ...) {
    va_list ap;
    va_start(ap, fmt);
    verror_at(loc, fmt, ap);
}

void error_tok(Token *tok, char *fmt, ...) {
    va_list ap;
    va_start(ap, fmt);
    verror_at(tok->str, fmt, ap);
}

bool startswith(char *p, char *q) {
    return memcmp(p, q, strlen(q)) == 0;
}

char *starts_with_reserved(char *p) {
    // Keyword
    char *kw[] = { "return", "if", "else", "while", "for", "int" };

    for (int i = 0; i < sizeof(kw) / sizeof(*kw); i++) {
        int len = strlen(kw[i]);
        if (startswith(p, kw[i]) && !is_alnum(p[len]))
            return kw[i];
    }

    char *ops[] = { "==", "!=", "<=", ">=" };

    for (int i = 0; i < sizeof(ops) / sizeof(*ops); i++) {
        if (startswith(p, ops[i]))
                return ops[i];
    }

    return NULL;
}

// Returns token if the current token matches a given string
Token *peek(char *s) {
    if (token->kind != TK_RESERVED || strlen(s) != token->len || strncmp(token->str, s, token->len))
        return NULL;

    return token;
}

// Ensure current token is a given string
void expect(char *s) {
    if (!peek(s))
        error_tok(token, "expected \"%s\"", s);

    token = token->next;
}

// Ensure that the current token is TK_IDENT
char *expect_ident() {
    if (token->kind != TK_IDENT)
        error_at(token->str, "expected an identifier");
    char *s = strndup(token->str, token->len);
    token = token->next;
    return s;
}

// 次のトークンが期待している記号のときにはトークンを1つ読み進めて
// 真を返す。それ以外のときには偽を返す
Token *consume(char *op) {
    if (token->kind != TK_RESERVED || strlen(op) != token->len || memcmp(token->str, op, token->len))
        return NULL;

    Token *t = token;
    token = token->next;

    return t;
}

Token *consume_ident(void) {
    if (token->kind != TK_IDENT)
        return NULL;

    Token *t = token;
    token = token->next;
    return t;
}

