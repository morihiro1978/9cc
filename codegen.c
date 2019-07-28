#include "9cc.h"

/* ラベルカウター */
static int label_count = 0;

/* 変数のコードを生成 */
static void gen_lvar(Node *node) {
    if (node->kind != ND_LVAR) {
        error("代入の左辺値が変数ではありません。");
    }
    printf("    mov rax, rbp\n");
    printf("    sub rax, %d\n", node->v.lvar.offset);
    printf("    push rax\n");
}

/* 抽象構文木を下りながらコードを生成 */
void gen(Node *node) {
    int cnt = label_count;
    label_count++;

    switch (node->kind) {
    case ND_NUM:
        printf("    push %d\n", node->v.num.val);
        return;
    case ND_ASSIGN:
        gen_lvar(node->v.op2.lhs);
        gen(node->v.op2.rhs);
        printf("    pop rdi\n");
        printf("    pop rax\n");
        printf("    mov [rax], rdi\n");
        printf("    push rdi\n");
        return;
    case ND_LVAR:
        gen_lvar(node);
        printf("    pop rax\n");
        printf("    mov rax, [rax]\n");
        printf("    push rax\n");
        return;
    case ND_RETURN:
        gen(node->v.op1.expr);
        printf("    pop rax\n");
        printf("    mov rsp, rbp\n");
        printf("    pop rbp\n");
        printf("    ret\n");
        return;
    case ND_IF:
        gen(node->v.cif.test);
        printf("    pop rax\n");
        printf("    cmp rax, 0\n");
        printf("    je .Lelse%d\n", cnt);
        gen(node->v.cif.tbody);
        printf("    jmp .Lend%d\n", cnt);
        printf(".Lelse%d:\n", cnt);
        if (node->v.cif.ebody == NULL) {
            // if-thenが実行されなかったときに何もpushしないと、
            // 次のpopでスタックがアンダーフローするため、ダミーpushする。
            printf("    push 0\n");
        } else {
            gen(node->v.cif.ebody);
        }
        printf(".Lend%d:\n", cnt);
        return;
    case ND_WHILE:
        printf(".Lbegin%d:\n", cnt);
        gen(node->v.cwhile.test);
        printf("    pop rax\n");
        printf("    cmp rax, 0\n");
        printf("    je .Lbreak%d\n", cnt);
        gen(node->v.cwhile.body);
        printf("    jmp .Lbegin%d\n", cnt);
        printf(".Lbreak%d:\n", cnt);
        // if-thenが実行されなかったときに何もpushしないと、
        // 次のpopでスタックがアンダーフローするため、ダミーpushする。
        printf("    push 0\n");
        printf(".Lend%d:\n", cnt);
        return;
    default:
        break;
    }

    gen(node->v.op2.lhs);
    gen(node->v.op2.rhs);
    printf("    pop rdi\n");
    printf("    pop rax\n");

    switch (node->kind) {
    case ND_ADD:  // +
        printf("    add rax, rdi\n");
        break;
    case ND_SUB:  // -
        printf("    sub rax, rdi\n");
        break;
    case ND_MUL:  // *
        printf("    imul rax, rdi\n");
        break;
    case ND_DIV:  // /
        printf("    cqo\n");
        printf("    idiv rdi\n");
        break;
    case ND_EQ:  // ==
        printf("    cmp rax, rdi\n");
        printf("    sete al\n");
        printf("    movzb rax, al\n");
        break;
    case ND_NE:  // !=
        printf("    cmp rax, rdi\n");
        printf("    setne al\n");
        printf("    movzb rax, al\n");
        break;
    case ND_LT:  // <
        printf("    cmp rax, rdi\n");
        printf("    setl al\n");
        printf("    movzb rax, al\n");
        break;
        break;
    case ND_LE:  // <=
        printf("    cmp rax, rdi\n");
        printf("    setle al\n");
        printf("    movzb rax, al\n");
        break;
    default:
        error("未定義のノードです。");
        break;
    }

    printf("    push rax\n");
}