#include "9cc.h"

// 現在着目しているToken
Token *token;

Node *parse(char *user_input) {
    // トークナイズする
    token = tokenize(user_input);
    Node *node = expr();
    program();
    return node;
}

void error(char *fmt, ...) {
    va_list ap;
    va_start(ap, fmt);
    vfprintf(stderr, fmt, ap);
    fprintf(stderr, "\n");
    exit(1);
}

void error_at(char *loc, char *fmt, ...) {
    va_list ap;
    va_start(ap, fmt);

    int pos = loc - user_input;
    fprintf(stderr, "%s\n", user_input);
    fprintf(stderr, "%*s", pos, ""); // pos個の空白を出力
    fprintf(stderr, "^ ");
    vfprintf(stderr, fmt, ap);
    fprintf(stderr, "\n");
    exit(1);
}

// 次のトークンが期待している記号のときにはトークンを1つ読み進めて
// 真を返す。それ以外のときには偽を返す
bool consume(char *op) {
    if (token->kind != TK_RESERVED || strlen(op) != token->len || memcmp(token->str, op, token->len))
        return false;
    token = token->next;
    return true;
}

Token *consume_ident(void) {
    if (token->kind != TK_IDENT)
        return NULL;

    Token *t = token;
    token = token->next;
    return t;
}

void expect(char *op) {
    if (token->kind != TK_RESERVED || strlen(op) != token->len || memcmp(token->str, op, token->len))
        error_at(token->str, "'%s'ではありません", op);
    token = token->next;
}

int expect_number() {
    if (token->kind != TK_NUM)
        error_at(token->str, "数ではありません");
    int val = token->val;
    token = token->next;
    return val;
}

bool at_eof() {
    return token->kind == TK_EOF;
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

bool startswith(char *p, char *q) {
    return memcmp(p, q, strlen(q)) == 0;
}

Token *tokenize(char *p) {
    Token head;
    head.next = NULL;
    Token *cur = &head;

    while (*p) {
        if (isspace(*p)) {
            p++;
            continue;
        }

        if (startswith(p, "==") || startswith(p, "!=") ||
            startswith(p, "<=") || startswith(p, ">=")) {
            cur = new_token(TK_RESERVED, cur, p, 2);
            p += 2;
            continue;
        }

        if (strchr("+-*/()<>", *p)) {
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

        if ('a' <= *p && *p <= 'z') {
            cur = new_token(TK_IDENT, cur, p++, 1);
            cur->len = 1;
            continue;
        }

        if (ispunct(*p)) {
            cur = new_token(TK_RESERVED, cur, p++, 1);
            continue;
        }

        error("トークナイズできません");
    }

    new_token(TK_EOF, cur, p, 0);
    return head.next;
}

Node *new_node(NodeKind kind, Node *lhs, Node *rhs) {
    Node *node = calloc(1, sizeof(Node));
    node->kind = kind;
    node->lhs = lhs;
    node->rhs = rhs;
    return node;
}

Node *new_node_num(int val) {
    Node *node = calloc(1, sizeof(Node));
    node->kind = ND_NUM;
    node->val = val;
    return node;
}

Node *assign() {
    Node *node = equality();
    if (consume("="))
        node = new_node(ND_ASSIGN, node, assign());
    return node;
}

Node *expr() {
    return assign();
}

Node *equality() {
    Node *node = relational();

    for(;;) {
        if (consume("=="))
            node = new_node(ND_EQ, node, relational());
        else if (consume("!="))
            node = new_node(ND_NE, node, relational());
        else
            return node;
    }
}

Node *relational() {
    Node *node = add();

    for(;;) {
        if (consume("<"))
            node = new_node(ND_LT, node, add());
        else if (consume("<="))
            node = new_node(ND_LE, node, add());
        else if (consume(">"))
            node = new_node(ND_LT, add(), node);
        else if (consume(">="))
            node = new_node(ND_LE, add(), node);
        else
            return node;
    }
}

Node *add() {
    Node *node = mul();

    for(;;) {
        if (consume("+"))
            node = new_node(ND_ADD, node, mul());
        else if (consume("-"))
            node = new_node(ND_SUB, node, mul());
        else
            return node;
    }

}

Node *mul() {
    Node *node = unary();

    for(;;) {
        if (consume("*"))
            node = new_node(ND_MUL, node, unary());
        else if (consume("/"))
            node = new_node(ND_DIV, node, unary());
        else
            return node;
    }
}

Node *primary() {
    if (consume("(")) {
        Node *node = expr();
        expect(")");
        return node;
    }

    Token *tok = consume_ident();
    if (tok) {
        Node *node = calloc(1, sizeof(Node));
        node->kind = ND_LVAR;
        node->offset = (tok->str[0] - 'a' + 1) * 8;
        return node;
    }

    return new_node_num(expect_number());
}

Node *unary() {
    if (consume("+"))
        return primary();
    if (consume("-"))
        return new_node(ND_SUB, new_node_num(0), primary());
    return primary();
}

Node *stmt() {
    Node *node = expr();
    expect(";");
    return node;
}

void program() {
    int i = 0;
    while(!at_eof())
        code[i++] = stmt();
    code[i] = NULL;
}
