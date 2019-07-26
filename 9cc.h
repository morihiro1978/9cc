#include <ctype.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* トークンの種類 */
typedef enum {
    TK_RESERVED,  // 記号
    TK_VAR,       // 変数
    TK_NUM,       // 整数
    TK_EOF        // EOF
} TokenKind;

/* トークン */
typedef struct Token Token;
struct Token {
    TokenKind kind;   // トークンの種類
    const char *str;  // トークン文字列
    int len;          // トークンの長さ
    int num;          // 数値: kind が TK_NUM の場合
                      // オフセット: kind が TK_VAR の場合
    struct {
        Token *next;
    } list;  // リスト
};

/* 抽象構文機のノードの種類 */
typedef enum {
    ND_ADD,     // +
    ND_SUB,     // -
    ND_MUL,     // *
    ND_DIV,     // /
    ND_EQ,      // ==
    ND_NE,      // !=
    ND_LT,      // <
    ND_LE,      // <=
    ND_NUM,     // 整数
    ND_ASSIGN,  // 代入
    ND_LVAR     // 変数
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
extern const char *user_input;

/* エラー出力関数 */
void error(char *fmt, ...);

/* エラー箇所を報告する */
void error_at(const char *exp, char *fmt, ...);

/* トークナイズする */
void tokenize(char *exp);

/* パース */
Node *parse(void);

/* 抽象構文木を下りながらコードを生成 */
void gen(Node *node);