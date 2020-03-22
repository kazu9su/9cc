#include "9cc.h"

// ローカル変数
LVar *locals;

// 変数を名前で検索する。見つからなかった場合はNULLを返す
LVar *find_lvar(Token *tok) {
    for (LVar *var = locals; var; var = var->next)
        if (strlen(var->name) == tok->len && !memcmp(tok->str, var->name, tok->len))
            return var;
    return NULL;
}

// 現在着目しているToken
Token *token;

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

Node *new_node(NodeKind kind) {
    Node *node = calloc(1, sizeof(Node));
    node->kind = kind;

    return node;
}

Node *new_unary(NodeKind kind, Node *expr) {
    Node *node = new_node(kind);
    node->lhs = expr;

    return node;
}

Node *new_binary(NodeKind kind, Node *lhs, Node *rhs) {
    Node *node = new_node(kind);
    node->lhs = lhs;
    node->rhs = rhs;

    return node;
}

Node *new_node_num(int val) {
    Node *node = new_node(ND_NUM);
    node->val = val;
    return node;
}

Node *new_lvar_node(LVar *lvar) {
    Node *node = new_node(ND_LVAR);
    node->lvar = lvar;
    return node;
}

LVar *new_lvar(char *name) {
    LVar *lvar = calloc(1, sizeof(LVar));
    lvar->next = locals;
    lvar->name = name;
    locals = lvar;

    return lvar;
}

Node *stmt(void);
Node *expr(void);
Node *assign(void);
Node *equality(void);
Node *relational(void);
Node *add(void);
Node *mul(void);
Node *unary(void);
Node *primary(void);
Function *function(void);

// program = function*
Function *program() {
    Function head = {};
    Function *cur = &head;

    while (!at_eof()) {
        cur->next = function();
        cur = cur->next;
    }

    return head.next;
}

Function *function() {
    locals = NULL;

    char *name = expect_ident();
    expect("(");
    expect(")");
    expect("{");

    Node head = {};
    Node *cur = &head;

    while(!consume("}")) {
        cur->next = stmt();
        cur = cur->next;
    }

    Function *fn = calloc(1, sizeof(Function));
    fn->name = name;
    fn->node = head.next;
    fn->locals = locals;

    return fn;
}

// stmt = "return" expr ";"
//      | "{" stmt* "}"
//      | "if" "(" expr ")" stmt ("else" stmt)?
//      | "while" "(" expr ")" stmt
//      | "for" "(" expr? ";" expr? ";" expr? ";" ")" stmt
//      | expr ";"
Node *stmt() {
    if (consume("return")) {
        Node *node = new_unary(ND_RETURN, expr());
        expect(";");

        return node;
    }

    if (consume("{")) {
        Node head = {};
        Node *cur = &head;

        while (!consume("}")) {
            cur->next = stmt();
            cur = cur->next;
        }

        Node *node = new_node(ND_BLOCK);
        node->body = head.next;

        return node;
    }

    if (consume("if")) {
        Node *node = new_node(ND_IF);
        expect("(");
        node->cond = expr();
        expect(")");
        node->then = stmt();

        if (consume("else"))
            node->els = stmt();

        return node;
    }

    if (consume("while")) {
        Node *node = new_node(ND_WHILE);
        expect("(");
        node->cond = expr();
        expect(")");
        node->then = stmt();

        return node;
    }

    if (consume("for")) {
        Node *node = new_node(ND_FOR);
        expect("(");

        if (!consume(";")) {
            node->init = expr();
            expect(";");
        }

        if (!consume(";")) {
            node->cond = expr();
            expect(";");
        }

        if (!consume(")")) {
            node->inc = expr();
            expect(")");
        }

        node->then = stmt();

        return node;
    }

    Node *node = new_unary(ND_EXPR_STMT, expr());
    expect(";");

    add_type(node);

    return node;
}

Node *expr() {
    return assign();
}

Node *assign() {
    Node *node = equality();
    if (consume("="))
        node = new_binary(ND_ASSIGN, node, assign());
    return node;
}

Node *equality() {
    Node *node = relational();

    for(;;) {
        if (consume("=="))
            node = new_binary(ND_EQ, node, relational());
        else if (consume("!="))
            node = new_binary(ND_NE, node, relational());
        else
            return node;
    }
}

Node *relational() {
    Node *node = add();

    for(;;) {
        if (consume("<"))
            node = new_binary(ND_LT, node, add());
        else if (consume("<="))
            node = new_binary(ND_LE, node, add());
        else if (consume(">"))
            node = new_binary(ND_LT, add(), node);
        else if (consume(">="))
            node = new_binary(ND_LE, add(), node);
        else
            return node;
    }
}

Node *add() {
    Node *node = mul();

    for(;;) {
        if (consume("+"))
            node = new_binary(ND_ADD, node, mul());
        else if (consume("-"))
            node = new_binary(ND_SUB, node, mul());
        else
            return node;
    }

}

Node *mul() {
    Node *node = unary();

    for(;;) {
        if (consume("*"))
            node = new_binary(ND_MUL, node, unary());
        else if (consume("/"))
            node = new_binary(ND_DIV, node, unary());
        else
            return node;
    }
}

Node *unary() {
    if (consume("+"))
        return unary();
    if (consume("-"))
        return new_binary(ND_SUB, new_node_num(0), unary());
    if (consume("&"))
        return new_unary(ND_ADDR, unary());
    if (consume("*"))
        return new_unary(ND_DEREF, unary());
    return primary();
}

// func_args = "(" (assign ("," assign)*)? ")"
Node *func_args() {
    if (consume(")"))
        return NULL;

    Node *head = assign();
    Node *cur = head;
    while(consume(",")) {
        cur->next = assign();
        cur = cur->next;
    }
    expect(")");

    return head;
}

// primary = num
//         | ident func_args?
//         | "(" expr ")"
Node *primary() {
    if (consume("(")) {
        Node *node = expr();
        expect(")");
        return node;
    }

    Token *tok = consume_ident();
    if (tok) {
        // Funcall
        if (consume("(")) {
            Node *node = new_node(ND_FUNCALL);
            node->funcname = strndup(tok->str, tok->len);
            node->args = func_args();

            return node;
        }

        // Variable
        LVar *lvar = find_lvar(tok);
        if (!lvar)
            lvar = new_lvar(strndup(tok->str, tok->len));

        return new_lvar_node(lvar);
    }

    return new_node_num(expect_number());
}


