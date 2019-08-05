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

    // アセンブリの前半を出力
    printf(".intel_syntax noprefix\n");
    printf(".global main\n");
    printf("main:\n");

    // プロローグ
    // 変数の領域を確保
    printf("# prologue\n");
    printf("    push rbp\n");
    printf("    mov rbp, rsp\n");
#if 0
    printf("    sub rsp, %d\n", program->v.block.tobal_local * 8);
#else
    for (int i = 0; i < program->v.block.total_local; i++) {
        printf("    push 0xcc\n");
    }
#endif
    printf("\n");

    // 全ての式を処理
    Node **code = program->v.block.code;
    for (int i = 0; code[i] != NULL; i++) {
        // 抽象構文木を下りながらコードを生成
        gen(code[i]);

        // スタックの一番上に式全体の結果が残っているので、それを戻り値とする
        printf("    pop rax\n");
        printf("\n");
    }

    // エピローグ
    // 最後の式の結果が RAX に残っているので、それを返す
    printf("# epilogue\n");
    printf("    mov rsp, rbp\n");
    printf("    pop rbp\n");
    printf("    ret\n");
    return 0;
}
