#include "9cc.h"

int main(int argc, char **argv) {
    if (argc != 2) {
        error("引数の個数が正しくありません");
    }

    // トークナイズしてパースする
    // 結果はcodeに保存される
    user_input = argv[1];
    token = tokenize(user_input);
    Function *prog = program();

    int offset = 0;
    for (LVar *lvar = prog->locals; lvar; lvar = lvar->next) {
        offset += 8;
        lvar->offset = offset;
    }

    prog->stack_size = offset;

    codegen(prog);

    return 0;
}
