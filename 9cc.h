#include <ctype.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef enum {
    ND_ADD, // +
    ND_SUB, // -
    ND_MUL, // *
    ND_DIV, // /
    ND_ASSIGN, // =
    ND_EQ, // ==
    ND_NE, // !=
    ND_LT, // >
    ND_LE, // <
    ND_LVAR, // ローカル変数
    ND_NUM, // 整数
} NodeKind;

typedef struct Node Node;

struct Node {
    NodeKind kind;
    Node *lhs; // 左辺
    Node *rhs; // 右辺
    int val; // kindがND_NUMの場合のみ使う
    int offset; // kindがND_LVARの場合のみ使う
};

// トークンの種類
typedef enum {
    TK_RESERVED, // 記号
    TK_IDENT, // 識別子
    TK_NUM, // 整数トークン
    TK_EOF, // 入力の終わりを表すトークン
} TokenKind;

typedef struct Token Token;

// トークン型
struct Token {
    TokenKind kind;
    Token *next;
    int val;
    char *str;
    int len;
};

void error(char *fmt, ...);
char *user_input;
void error_at(char *loc, char *fmt, ...);
bool consume(char *op);
Token *consume_ident(void);
void expect(char *op);
int expect_number();
bool at_eof();
Token *new_token(TokenKind kind, Token *cur, char *str, int len);
bool startswith(char *p, char *q);
Token *tokenize(char *p);
Node *new_node(NodeKind kind, Node *lhs, Node *rhs);
Node *new_node_num(int val);
Node *expr();
Node *mul();
Node *primary();
Node *unary();
Node *add();
Node *relational();
Node *equality();
Node *assign();

char *user_input;

void gen(Node *node);
void codegen(Node *node);
void program();
Node *parse(char *user_input);

Node *code[100];
extern Token *token;
