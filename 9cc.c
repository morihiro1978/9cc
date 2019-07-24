/* BNF:
   expr  = mul ("+" mul | "-" mul)*
   mul   = unary ("*" unary | "/" unary)*
   unary = ("+" | "-")? term
   term  = num | "(" expr ")"
 */
#include <ctype.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

/* トークンの種類 */
typedef enum {
    TK_RESERVED,  // 記号
    TK_NUM,       // 整数
    TK_EOF        // EOF
} TokenKind;

/* トークン */
typedef struct Token Token;
struct Token {
    TokenKind kind;   // トークンの種類
    const char *str;  // トークン文字列
    int num;          // 数値 (kind が TK_NUM の場合のみ有効)
    struct {
        Token *next;
    } list;  // リスト
};

/* 現在のトークン */
static Token *token = NULL;

/* 抽象構文機のノードの種類 */
typedef enum {
    ND_ADD,  // +
    ND_SUB,  // -
    ND_MUL,  // *
    ND_DIV,  // /
    ND_NUM   // 整数
} NodeKind;

/* 抽象構文機のノード */
typedef struct Node Node;
struct Node {
    NodeKind kind;  // ノードの種類
    Node *lhs;      // 左辺
    Node *rhs;      // 右辺
    int num;        // 数値 (kind が ND_NUM の場合のみ有効)
};

/* 入力プログラム */
static const char *user_input = NULL;

/* エラー出力関数 */
static void error(char *fmt, ...) {
    va_list ap;
    va_start(ap, fmt);
    vfprintf(stderr, fmt, ap);
    fprintf(stderr, "\n");
    exit(1);
}

/* エラー箇所を報告する */
static void error_at(const char *exp, char *fmt, ...) {
    va_list ap;
    va_start(ap, fmt);

    int pos = exp - user_input;
    fprintf(stderr, "%s\n", user_input);
    fprintf(stderr, "%*s", pos, "");
    fprintf(stderr, "^ ");
    vfprintf(stderr, fmt, ap);
    fprintf(stderr, "\n");
    exit(1);
}

/* トークンの種類を文字列に変換する */
static const char *tokenKind_to_str(TokenKind kind) {
    switch (kind) {
    case TK_RESERVED:
        return "TK_RESERVED";
    case TK_NUM:
        return "TK_NUM";
    case TK_EOF:
        return "TK_EOF";
    default:
        return "Undefiend";
    }
}

/* トークンを表示する */
static void print_token(const Token *tok) {
    printf(
        "Token: 0x%08lx\n"
        " .kind     : %s\n"
        " .str      : %s\n"
        " .num      : %d\n"
        " .list.next: 0x%08lx\n",
        (uintptr_t)tok, tokenKind_to_str(tok->kind), tok->str, tok->num,
        (uintptr_t)tok->list.next);
}

/* トークンリストを表示する */
static void print_token_list(const Token *head) {
    Token *tok = (Token *)head;
    while (tok != NULL) {
        print_token(tok);
        tok = tok->list.next;
    }
}

/* 文字が記号か？ */
static bool is_exp_reserved(const char c) {
    switch (c) {
    case '+':  // fall down
    case '-':  // fall down
    case '*':  // fall down
    case '/':  // fall down
    case '(':  // fall down
    case ')':  // fall down
        return true;
    }
    return false;
}

/* 文字列から新しいトークンを作成し、cur リストに追加する */
static Token *new_token(TokenKind kind, const char *exp, Token *cur) {
    Token *tok = calloc(1, sizeof(Token));

    tok->kind = kind;
    tok->str = exp;
    cur->list.next = tok;
    return tok;
}

/* トークナイズする */
static void tokenize(char *exp) {
    Token head;
    head.list.next = NULL;
    Token *cur = &head;

    while (1) {
        // EOF
        if (exp[0] == '\0') {
            cur = new_token(TK_EOF, exp, cur);
            break;
        }
        // 空白をスキップ
        else if (isspace(exp[0]) != 0) {
            exp++;
        }
        // 記号
        else if (is_exp_reserved(exp[0]) == true) {
            cur = new_token(TK_RESERVED, exp, cur);
            exp++;
        }
        // 数値
        else if (isdigit(exp[0])) {
            cur = new_token(TK_NUM, exp, cur);
            cur->num = strtol(exp, &exp, 10);
        }
        // 未知のトークン
        else {
            error_at(exp, "トークナイズできません。");
        }
    }
    token = head.list.next;
}

/* トークンが指定の記号なら true を返し、トークンを進める。
   別の記号なら false を返す。
 */
static bool consume(char mark) {
    if ((token->kind != TK_RESERVED) || (token->str[0] != mark)) {
        return false;
    }
    token = token->list.next;
    return true;
}

/* トークンが指定の記号なら、トークンを進める。
   違っていたらパニックする。
*/
static void expect(char mark) {
    if (consume(mark) == false) {
        error_at(token->str, "予期せぬトークンです");
    }
}

/* トークンが数値ならそれを返し、トークンを進める。
   違っていたらパニックする。
 */
static int expect_number(void) {
    if (token->kind != TK_NUM) {
        error_at(token->str, "数値ではありません");
    }
    int num = token->num;
    token = token->list.next;
    return num;
}

/* EOFか？ */
static bool eof(void) {
    if (token->kind != TK_EOF) {
        return false;
    }
    return true;
}

/* 2項のノードを作成する */
struct Node *new_node(NodeKind kind, Node *lhs, Node *rhs) {
    Node *node = calloc(1, sizeof(Node));

    node->kind = kind;
    node->lhs = lhs;
    node->rhs = rhs;
    return node;
}

/* 数値ノードを作成する */
struct Node *new_node_num(int num) {
    Node *node = calloc(1, sizeof(Node));

    node->kind = ND_NUM;
    node->num = num;
    return node;
}

/* パーサ: num */
static Node *num(void) {
    return new_node_num(expect_number());
}

/* パーサ: term */
static Node *expr(void);
static Node *term(void) {
    if (consume('(') == true) {
        Node *node = expr();
        expect(')');
        return node;
    }
    return num();
}

/* パーサ: unary */
static Node *unary(void) {
    if (consume('+') == true) {
        return term();
    } else if (consume('-') == true) {
        return new_node(ND_SUB, new_node_num(0), term());
    } else {
        return term();
    }
}

/* パーサ: mul */
static Node *mul(void) {
    Node *node = unary();

    while (1) {
        if (consume('*') == true) {
            node = new_node(ND_MUL, node, unary());
        } else if (consume('/') == true) {
            node = new_node(ND_DIV, node, unary());
        } else {
            break;
        }
    }
    return node;
}

/* パーサ: expr */
static Node *expr(void) {
    Node *node = mul();

    while (1) {
        if (consume('+') == true) {
            node = new_node(ND_ADD, node, mul());
        } else if (consume('-') == true) {
            node = new_node(ND_SUB, node, mul());
        } else {
            break;
        }
    }
    return node;
}

/* パース */
static Node *parse(void) {
    return expr();
}

/* 抽象構文木を下りながらコードを生成 */
static void gen(Node *node) {
    if (node->kind == ND_NUM) {
        printf("    push %d\n", node->num);
        return;
    }
    gen(node->lhs);
    gen(node->rhs);

    printf("    pop rdi\n");
    printf("    pop rax\n");

    switch (node->kind) {
    case ND_ADD:
        printf("    add rax, rdi\n");
        break;
    case ND_SUB:
        printf("    sub rax, rdi\n");
        break;
    case ND_MUL:
        printf("    imul rax, rdi\n");
        break;
    case ND_DIV:
        printf("    cqo\n");
        printf("    idiv rdi\n");
        break;
    default:
        error("未定義のノードです。");
        break;
    }

    printf("    push rax\n");
}

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

    // 抽象構文木を下りながらコードを生成
    gen(node);

    // スタックの一番上に式全体の結果が残っているので、それを戻り値とする
    printf("    pop rax\n");
    printf("    ret\n");
    return 0;
}
