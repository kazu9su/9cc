#include "9cc.h"

static int labelseq = 1;
static char *funcname;
static char *argreg[] = { "rdi", "rsi", "rdx", "rcx", "r8", "r9" };

void gen_lval(Node *node) {
    switch (node->kind) {
        case ND_LVAR:
            printf("  lea rax, [rbp-%d]\n", node->lvar->offset);
            printf("  push rax\n");
            return;
        case ND_DEREF:
            gen(node->lhs);

            return;
    }

    error("代入の左辺値が変数ではありません");
}

void load() {
    printf("  pop rax\n");
    printf("  mov rax, [rax]\n");
    printf("  push rax\n");
}

void store() {
    printf("  pop rdi\n");
    printf("  pop rax\n");
    printf("  mov [rax], rdi\n");
    printf("  push rdi\n");
}

void gen(Node *node) {
    switch (node->kind) {
        case ND_NULL:
            return;
        case ND_NUM:
            printf("  push %d\n", node->val);
            return;
        case ND_EXPR_STMT:
            gen(node->lhs);
            printf("  add rsp, 8\n");
            return;
        case ND_LVAR:
            gen_lval(node);
            load();
            return;
        case ND_ASSIGN:
            gen_lval(node->lhs);
            gen(node->rhs);
            store();
            return;
        case ND_ADDR:
            gen_lval(node->lhs);
            return;
        case ND_DEREF:
            gen(node->lhs);
            load();

            return;
        case ND_RETURN:
            gen(node->lhs);
            printf("  pop rax\n");
            printf("  jmp .L.return.%s\n", funcname);
            return;
        case ND_BLOCK:
            for (Node *n = node->body; n; n = n->next) {
                gen(n);
            }

            return;
        case ND_FUNCALL: {
            int nargs = 0;
            for (Node *arg = node->args; arg; arg = arg->next) {
                gen(arg);
                nargs++;
            }

            for (int i = nargs - 1; i >= 0; i--)
                printf("  pop %s\n", argreg[i]);

            // We need to align RSP to a 16 byte boundary before
            // calling a function because it is an ABI requirement.
            // RAX is set to 0 for variadic function
            int seq = labelseq++;
            printf("  mov rax, rsp\n");
            printf("  and rax, 15\n"); // check bitwise AND operation
            printf("  jnz .L.call.%d\n", seq); // if 8 bytes, adjust to 16 bytes
            printf("  mov rax, 0\n");
            printf("  call %s\n", node->funcname);
            printf("  jmp .L.end.%d\n", seq);
            printf(".L.call.%d:\n", seq);
            printf("  sub rsp, 8\n");
            printf("  mov rax, 0\n");
            printf("  call %s\n", node->funcname);
            printf("  add rsp, 8\n");
            printf(".L.end.%d:\n", seq);

            printf("  push rax\n");
            return;
        }
        case ND_IF: {
            int seq = labelseq++;

            if (node->els) {
                gen(node->cond);
                printf("  pop rax\n");
                printf("  cmp rax, 0\n");
                printf("  je .L.else.%d\n", seq);
                gen(node->then);
                printf("  jmp .L.end.%d\n", seq);
                printf(".L.else.%d:\n", seq);
                gen(node->els);
                printf(".L.end.%d:\n", seq);
            } else {
                gen(node->cond);
                printf("  pop rax\n");
                printf("  cmp rax, 0\n");
                printf("  je .L.end.%d\n", seq);
                gen(node->then);
                printf(".L.end.%d:\n", seq);
            }

            return;
        }
        case ND_WHILE: {
            int seq = labelseq++;

            printf(".L.begin.%d:\n", seq);
            gen(node->cond);
            printf("  pop rax\n");
            printf("  cmp rax, 0\n");
            printf("  je .L.end.%d\n", seq);
            gen(node->then);
            printf("  jmp .L.begin.%d\n", seq);
            printf(".L.end.%d:\n", seq);

            return;
        }
        case ND_FOR: {
            int seq = labelseq++;

            if (node->init)
                gen(node->init);

            printf(".L.begin.%d:\n", seq);

            if (node->cond) {
                gen(node->cond);
                printf("  pop rax\n");
                printf("  cmp rax, 0\n");
                printf("  je .L.end.%d\n", seq);
            }

            gen(node->then);

            if (node->inc)
                gen(node->inc);

            printf("  jmp .L.begin.%d\n", seq);
            printf(".L.end.%d:\n", seq);

            return;
        }
    }

    gen(node->lhs);
    gen(node->rhs);

    printf("  pop rdi\n");
    printf("  pop rax\n");

    switch (node->kind) {
        case ND_ADD:
            printf("  add rax, rdi\n");
            break;
        case ND_SUB:
            printf("  sub rax, rdi\n");
            break;
        case ND_MUL:
            printf("  imul rax, rdi\n");
            break;
        case ND_EQ:
            printf("  cmp rax, rdi\n");
            printf("  sete al\n");
            printf("  movzb rax, al\n");
            break;
        case ND_NE:
            printf("  cmp rax, rdi\n");
            printf("  setne al\n");
            printf("  movzb rax, al\n");
            break;
        case ND_LT:
            printf("  cmp rax, rdi\n");
            printf("  setl al\n");
            printf("  movzb rax, al\n");
            break;
         case ND_LE:
            printf("  cmp rax, rdi\n");
            printf("  setle al\n");
            printf("  movzb rax, al\n");
            break;
       case ND_DIV:
            printf("  cqo\n");
            printf("  idiv rdi\n");
            break;

    }

    printf("  push rax\n");
}

void codegen(Function *prog) {
    // アセンブリの前半部分を出力
    printf(".intel_syntax noprefix\n");

    for (Function *fn = prog; fn; fn = fn->next) {
        printf(".global %s\n", fn->name);
        printf("%s:\n", fn->name);

        funcname = fn->name;

        // プロローグ
        printf("  push rbp\n");
        printf("  mov rbp, rsp\n");
        printf("  sub rsp, %d\n", fn->stack_size);

        // 先頭の式から順にコード生成
        for (Node *node = fn->node; node; node = node->next)
            gen(node);

        // エピローグ
        // 最後の式の結果がRAXに残っているのでそれが返り値になる
        printf(".L.return.%s:\n", funcname);
        printf("  mov rsp, rbp\n");
        printf("  pop rbp\n");
        printf("  ret\n");
    }
}
