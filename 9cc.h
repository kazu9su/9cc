#define _GNU_SOURCE
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
    ND_RETURN, // return
    ND_IF, // if
    ND_EXPR_STMT, // Expression statement
    ND_WHILE, // while
    ND_FOR, // for
    ND_BLOCK, // block
    ND_FUNCALL, // Function call
} NodeKind;

// ローカル変数の型
typedef struct LVar LVar;

struct LVar {
    LVar *next; // 次の変数かNULL
    char *name; // 変数の名前
    int offset; // RBPからのオフセット
};

typedef struct Node Node;

struct Node {
    NodeKind kind;
    Node *next;
    Node *lhs; // 左辺
    Node *rhs; // 右辺

    // if or while or for statement
    Node *cond;
    Node *then;
    Node *els;
    Node *init;
    Node *inc;

    // Block
    Node *body;

    // Function call
    char *funcname;
    Node *args;

    LVar *lvar;
    int val; // kindがND_NUMの場合のみ使う
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
char *expect_ident();
bool at_eof();
bool startswith(char *p, char *q);
Token *tokenize(char *p);
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

LVar *find_lvar(Token *tok);

Node *code[100];
extern Token *token;

typedef struct Function Function;
struct Function {
    Function *next;
    char *name;
    Node *node;
    LVar *locals;
    int stack_size;
};

Function *program(void);

void codegen(Function *prog);
