#include <stdarg.h>
#include "9cc.h"

/* ラベルカウター */
static int label_count = 0;

static Node *cur_func = NULL;

static void comment(const char *format, ...) {
    va_list ap;
    printf("# ");
    va_start(ap, format);
    vprintf(format, ap);
    va_end(ap);
}

/* NULL */
static void gen_null(Node *node) {
    printf("    push 0xcc\n");
}

/* 整数 */
static void gen_num(Node *node) {
    comment("num: %d\n", node->v.num.val);
    printf("    push %d\n", node->v.num.val);
}

/* 足し算 */
static void gen_add(Node *node) {
    gen(node->v.op2.lhs);
    gen(node->v.op2.rhs);
    printf("    pop rdi\n");
    printf("    pop rax\n");
    printf("    add rax, rdi\n");
    printf("    push rax\n");
}

/* 引き算 */
static void gen_sub(Node *node) {
    gen(node->v.op2.lhs);
    gen(node->v.op2.rhs);
    printf("    pop rdi\n");
    printf("    pop rax\n");
    printf("    sub rax, rdi\n");
    printf("    push rax\n");
}

/* 掛け算 */
static void gen_mul(Node *node) {
    gen(node->v.op2.lhs);
    gen(node->v.op2.rhs);
    printf("    pop rdi\n");
    printf("    pop rax\n");
    printf("    imul rax, rdi\n");
    printf("    push rax\n");
}

/* 割り算 */
static void gen_div(Node *node) {
    gen(node->v.op2.lhs);
    gen(node->v.op2.rhs);
    printf("    pop rdi\n");
    printf("    pop rax\n");
    printf("    cqo\n");
    printf("    idiv rdi\n");
    printf("    push rax\n");
}

/* 比較演算子 */
static void gen_eq(Node *node) {
    gen(node->v.op2.lhs);
    gen(node->v.op2.rhs);
    printf("    pop rdi\n");
    printf("    pop rax\n");
    printf("    cmp rax, rdi\n");
    printf("    sete al\n");
    printf("    movzb rax, al\n");
    printf("    push rax\n");
}

/* 比較演算子 */
static void gen_ne(Node *node) {
    gen(node->v.op2.lhs);
    gen(node->v.op2.rhs);
    printf("    pop rdi\n");
    printf("    pop rax\n");
    printf("    cmp rax, rdi\n");
    printf("    setne al\n");
    printf("    movzb rax, al\n");
    printf("    push rax\n");
}

/* 比較演算子 */
static void gen_lt(Node *node) {
    gen(node->v.op2.lhs);
    gen(node->v.op2.rhs);
    printf("    pop rdi\n");
    printf("    pop rax\n");
    printf("    cmp rax, rdi\n");
    printf("    setl al\n");
    printf("    movzb rax, al\n");
    printf("    push rax\n");
}

/* 比較演算子 */
static void gen_le(Node *node) {
    gen(node->v.op2.lhs);
    gen(node->v.op2.rhs);
    printf("    pop rdi\n");
    printf("    pop rax\n");
    printf("    cmp rax, rdi\n");
    printf("    setle al\n");
    printf("    movzb rax, al\n");
    printf("    push rax\n");
}

/* 変数のアドレス */
static void gen_lvar_addr(Node *node) {
    if (node->kind != ND_LVAR) {
        error("代入の左辺値が変数ではありません。");
    }
    comment("lvar: %.*s \n", node->v.lvar.len, node->v.lvar.name);
    printf("    mov rax, rbp\n");
    printf("    sub rax, %d\n", node->v.lvar.offset);
    printf("    push rax\n");
}

/* 変数 */
static void gen_lvar(Node *node) {
    gen_lvar_addr(node);
    printf("    pop rax\n");
    printf("    mov rax, [rax]\n");
    printf("    push rax\n");
}

/* 代入 */
static void gen_assign(Node *node) {
    gen_lvar_addr(node->v.op2.lhs);
    gen(node->v.op2.rhs);
    comment("assign\n");
    printf("    pop rdi\n");
    printf("    pop rax\n");
    printf("    mov [rax], rdi\n");
    printf("    push rdi\n");
}

/* return */
static void gen_return(Node *node) {
    gen(node->v.op1.expr);
    comment("return\n");
    printf("    pop rax\n");
    /*
    printf("    mov rsp, rbp\n");
    printf("    pop rbp\n");
    printf("    ret\n");
     */
    printf("    jmp .Lret_%.*s\n",
           cur_func->v.deffunc.len,
           cur_func->v.deffunc.name);
}

/* if */
static void gen_if(Node *node) {
    int cnt = label_count;
    label_count++;

    comment("if - test -->\n");
    gen(node->v.cif.test);
    comment("if - test <--\n");
    printf("    pop rax\n");
    printf("    cmp rax, 0\n");
    printf("    je .Lelse%d\n", cnt);
    comment("if - tbody -->\n");
    gen(node->v.cif.tbody);
    comment("if - tbody <--\n");
    printf("    jmp .Lend%d\n", cnt);
    printf(".Lelse%d:\n", cnt);
    comment("if - ebody -->\n");
    gen(node->v.cif.ebody);
    comment("if - ebody <--\n");
    printf(".Lend%d:\n", cnt);
}

/* while */
static void gen_while(Node *node) {
    int cnt = label_count;
    label_count++;

    comment("while\n");
    printf(".Lbegin%d:\n", cnt);
    comment("while - test -->\n");
    gen(node->v.cwhile.test);
    comment("while - test <--\n");
    printf("    pop rax\n");
    printf("    cmp rax, 0\n");
    printf("    je .Lbreak%d\n", cnt);
    comment("while - body -->\n");
    gen(node->v.cwhile.body);
    comment("while - body <--\n");
    printf("    pop rax\n");
    printf("    jmp .Lbegin%d\n", cnt);
    printf(".Lbreak%d:\n", cnt);
    gen(NULL);  // dummy push
    printf(".Lend%d:\n", cnt);
}

/* for */
static void gen_for(Node *node) {
    int cnt = label_count;
    label_count++;

    comment("for - init -->\n");
    gen(node->v.cfor.init);
    comment("for - init <--\n");
    printf("    pop rax\n");
    printf(".Lbegin%d:\n", cnt);
    comment("for - test -->\n");
    gen(node->v.cfor.test);
    comment("for - test <--\n");
    printf("    pop rax\n");
    printf("    cmp rax, 0\n");
    printf("    je .Lbreak%d\n", cnt);
    comment("for - body -->\n");
    gen(node->v.cfor.body);
    comment("for - body <--\n");
    printf("    pop rax\n");
    comment("for - update -->\n");
    gen(node->v.cfor.update);
    comment("for - update <--\n");
    printf("    pop rax\n");
    printf("    jmp .Lbegin%d\n", cnt);
    printf(".Lbreak%d:\n", cnt);
    gen(NULL);  // dummy push
    printf(".Lend%d:\n", cnt);
}

/* 関数呼び出し */
static void gen_call_func(Node *node) {
    int i;
    const char *regs[] = {"rdi",
                          "rsi",
                          "rdx",
                          "rcx",
                          "r8",
                          "r9"};  // 第1～6引数に使用するレジスタ

    comment("func: %.*s\n", node->v.func.len, node->v.func.name);
    for (i = 0; i < node->v.func.num_param; i++) {
        gen(node->v.func.params[i]);
    }
    for (i = (node->v.func.num_param - 1); i >= 0; i--) {
        printf("    pop %s\n", regs[i]);
    }
    // 関数呼び出しのまえに、rspを16の倍数に整える
    printf("    push r12\n");
    printf("    mov r12, rsp\n");
    printf("    and r12, 0xf\n");
    printf("    sub rsp, r12\n");
    printf("    call %.*s\n", node->v.func.len, node->v.func.name);
    printf("    add rsp, r12\n");
    printf("    pop r12\n");
    printf("    push rax\n");
}

/* 関数定義 */
static void gen_define_func(Node *deffunc) {
    int i;
    const char *regs[] = {"rdi",
                          "rsi",
                          "rdx",
                          "rcx",
                          "r8",
                          "r9"};  // 第1～6引数に使用するレジスタ

    cur_func = deffunc;

    // 関数名
    printf("%.*s:\n", deffunc->v.deffunc.len, deffunc->v.deffunc.name);
    printf("    nop\n");  // アセンブリデバッグでブレイクポイントを貼るためのnp

    // プロローグ
    comment("prologue\n");

    // レジスタを退避
    printf("    push rbp\n");
    printf("    mov rbp, rsp\n");

    // パラメータを変数領域にセット
    for (i = 0; i < deffunc->v.deffunc.num_param; i++) {
        printf("    push %s\n", regs[i]);
    }

    // 変数の領域を確保
    for (; i < deffunc->v.deffunc.block->v.block.total_local; i++) {
        printf("    push 0xcc\n");
    }
    printf("\n");

    // ブロック内のコードを生成
    gen(deffunc->v.deffunc.block);

    // エピローグ
    comment("epilogue\n");
    printf(".Lret_%.*s:\n", deffunc->v.deffunc.len, deffunc->v.deffunc.name);
    printf("    mov rsp, rbp\n");
    printf("    pop rbp\n");
    printf("    ret\n");
}

/* ブロック */
static void gen_block(Node *block) {
    if (block->v.block.num_code > 0) {
        int i = 0;
        while (1) {
            gen(block->v.block.code[i]);
            i++;
            if (i >= block->v.block.num_code) {
                break;
            }
            // ステートメントごとに、そのステートメントが push した値を pop
            // する。 しかし、ブロックの最後のステートメントは、次の gen() で
            // pop される。
            printf("    pop rax\n");
        }
    }
}

/* アドレス取得 */
static void gen_addr(Node *node) {
    comment("&{var}\n");
    gen_lvar_addr(node->v.op1.expr);
}

/* 参照外し */
static void gen_deref(Node *node) {
    comment("*{var} (1/2)\n");
    gen(node->v.op1.expr);
    comment("*{var} (2/2)\n");
    printf("    pop rax\n");
    printf("    mov rax, [rax]\n");
    printf("    push rax\n");
}

/* 抽象構文木を下りながらコードを生成 */
void gen(Node *node) {
    if (node == NULL) {
        // 式が無いときに何もpushしないと、次のpopでスタックがアンダーフローするため、ダミーpushする。
        printf("    push 0xcc\n");
        return;
    }

    switch (node->kind) {
    case ND_NULL:
        gen_null(node);
        break;
    case ND_NUM:
        gen_num(node);
        break;
    case ND_ADD:
        gen_add(node);
        break;
    case ND_SUB:
        gen_sub(node);
        break;
    case ND_MUL:
        gen_mul(node);
        break;
    case ND_DIV:
        gen_div(node);
        break;
    case ND_EQ:
        gen_eq(node);
        break;
    case ND_NE:
        gen_ne(node);
        break;
    case ND_LT:
        gen_lt(node);
        break;
    case ND_LE:
        gen_le(node);
        break;
    case ND_LVAR:
        gen_lvar(node);
        break;
    case ND_ASSIGN:
        gen_assign(node);
        break;
    case ND_RETURN:
        gen_return(node);
        break;
    case ND_IF:
        gen_if(node);
        break;
    case ND_WHILE:
        gen_while(node);
        break;
    case ND_FOR:
        gen_for(node);
        break;
    case ND_BLOCK:
        gen_block(node);
        break;
    case ND_FUNC:
        gen_call_func(node);
        break;
    case ND_DEFFUNC:
        gen_define_func(node);
        break;
    case ND_ADDR:
        gen_addr(node);
        break;
    case ND_DEREF:
        gen_deref(node);
        break;
    default:
        error("未定義のノードです。");
        break;
    }
    printf("\n");
}