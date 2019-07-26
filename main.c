#include "9cc.h"

int main(int argc, char **argv) {
    if (argc != 2) {
        error("引数の個数が間違っています。");
        return 1;
    }

    // 式
    user_input = argv[1];

    // トークナイズする
    tokenize(argv[1]);
    // print_token_list(token);

    // パース
    Node *node = parse();

    // アセンブリの前半を出力
    printf(".intel_syntax noprefix\n");
    printf(".global main\n");
    printf("main:\n");

    // エピローグ
    // 変数 a ～ z までの領域を確保
    printf("    push rbp\n");
    printf("    mov rbp, rsp\n");
    printf("    sub rsp, 208\n");  // 26 * 8 = 208

    // 抽象構文木を下りながらコードを生成
    gen(node);

    // スタックの一番上に式全体の結果が残っているので、それを戻り値とする
    printf("    pop rax\n");

    // エピローグ
    // 最後の式の結果が RAX に残っているので、それを返す
    printf("    mov rsp, rbp\n");
    printf("    pop rbp\n");
    printf("    ret\n");
    return 0;
}