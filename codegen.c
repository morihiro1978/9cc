#include "9cc.h"

/* ラベルカウター */
static int label_count = 0;

/* 変数のコードを生成 */
static void gen_lvar(Node *node) {
    if (node->kind != ND_LVAR) {
        error("代入の左辺値が変数ではありません。");
    }
    printf("# lvar: %.*s \n", node->v.lvar.len, node->v.lvar.name);
    printf("    mov rax, rbp\n");
    printf("    sub rax, %d\n", node->v.lvar.offset);
    printf("    push rax\n");
    printf("\n");
}

/* ブロックのコードを生成 */
static void gen_block(Node *block) {
    if (block->v.block.num > 0) {
        int i = 0;
        while (1) {
            gen(block->v.block.code[i]);
            i++;
            if (i >= block->v.block.num) {
                break;
            }
            // ステートメントごとに、そのステートメントが push した値を pop
            // する。 しかし、ブロックの最後のステートメントは、次の gen() で
            // pop される。
            printf("    pop rax\n");
        }
    }
}

/* 抽象構文木を下りながらコードを生成 */
void gen(Node *node) {
    if (node == NULL) {
        // 式が無いときに何もpushしないと、次のpopでスタックがアンダーフローするため、ダミーpushする。
        printf("    push 0xcc\n");
        return;
    }

    int cnt = label_count;
    label_count++;

    switch (node->kind) {
    case ND_NUM:
        printf("# num: %d\n", node->v.num.val);
        printf("    push %d\n", node->v.num.val);
        printf("\n");
        return;
    case ND_ASSIGN:
        gen_lvar(node->v.op2.lhs);
        gen(node->v.op2.rhs);
        printf("# assign\n");
        printf("    pop rdi\n");
        printf("    pop rax\n");
        printf("    mov [rax], rdi\n");
        printf("    push rdi\n");
        printf("\n");
        return;
    case ND_LVAR:
        gen_lvar(node);
        printf("    pop rax\n");
        printf("    mov rax, [rax]\n");
        printf("    push rax\n");
        printf("\n");
        return;
    case ND_RETURN:
        gen(node->v.op1.expr);
        printf("# return\n");
        printf("    pop rax\n");
        printf("    mov rsp, rbp\n");
        printf("    pop rbp\n");
        printf("    ret\n");
        printf("\n");
        return;
    case ND_IF:
        printf("# if - test -->\n");
        gen(node->v.cif.test);
        printf("# if - test <--\n");
        printf("    pop rax\n");
        printf("    cmp rax, 0\n");
        printf("    je .Lelse%d\n", cnt);
        printf("# if - tbody -->\n");
        gen(node->v.cif.tbody);
        printf("# if - tbody <--\n");
        printf("    jmp .Lend%d\n", cnt);
        printf(".Lelse%d:\n", cnt);
        printf("# if - ebody -->\n");
        gen(node->v.cif.ebody);
        printf("# if - ebody <--\n");
        printf(".Lend%d:\n", cnt);
        printf("\n");
        return;
    case ND_WHILE:
        printf("# while\n");
        printf(".Lbegin%d:\n", cnt);
        printf("# while - test -->\n");
        gen(node->v.cwhile.test);
        printf("# while - test <--\n");
        printf("    pop rax\n");
        printf("    cmp rax, 0\n");
        printf("    je .Lbreak%d\n", cnt);
        printf("# while - body -->\n");
        gen(node->v.cwhile.body);
        printf("# while - body <--\n");
        printf("    pop rax\n");
        printf("    jmp .Lbegin%d\n", cnt);
        printf(".Lbreak%d:\n", cnt);
        gen(NULL);  // dummy push
        printf(".Lend%d:\n", cnt);
        printf("\n");
        return;
    case ND_FOR:
        printf("# for - init -->\n");
        gen(node->v.cfor.init);
        printf("# for - init <--n");
        printf("    pop rax\n");
        printf(".Lbegin%d:\n", cnt);
        printf("# for - test -->\n");
        gen(node->v.cfor.test);
        printf("# for - test <--n");
        printf("    pop rax\n");
        printf("    cmp rax, 0\n");
        printf("    je .Lbreak%d\n", cnt);
        printf("# for - body -->\n");
        gen(node->v.cfor.body);
        printf("# for - body <--n");
        printf("    pop rax\n");
        printf("# for - update -->\n");
        gen(node->v.cfor.update);
        printf("# for - update <--n");
        printf("    pop rax\n");
        printf("    jmp .Lbegin%d\n", cnt);
        printf(".Lbreak%d:\n", cnt);
        gen(NULL);  // dummy push
        printf(".Lend%d:\n", cnt);
        printf("\n");
        return;
    case ND_BLOCK:
        gen_block(node);
        printf("\n");
        return;
    case ND_FUNC:
        printf("# func: %.*s\n", node->v.func.len, node->v.func.name);
        printf("    push rbp\n");
        printf("    mov rbp, rsp\n");
        // 関数呼び出しのまえに、rspを16の倍数に整える
        printf("    mov r12, rsp\n");
        printf("    and r12, 0xf\n");
        printf("    sub rsp, r12\n");
        printf("    call %.*s\n", node->v.func.len, node->v.func.name);
        gen(NULL);  // dummy push
        printf("\n");
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
    printf("\n");
}