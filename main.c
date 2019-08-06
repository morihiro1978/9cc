#include "9cc.h"

int main(int argc, char **argv) {
    if (argc != 2) {
        error("引数の個数が間違っています。");
        return 1;
    }

    // 式をトークナイズしてパースする
    user_input = argv[1];
    tokenize(argv[1]);
    Node *program = parse();

    // コード出力
    printf(".intel_syntax noprefix\n");
    printf(".global main\n");
    gen(program);

    return 0;
}
