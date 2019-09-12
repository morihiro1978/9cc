#include <ctype.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* 組み込み型 */
typedef enum {
    TY_INT,     // int
} Type;

/* トークンの種類 */
typedef enum {
    TK_RESERVED,   // 記号
    TK_RETURN,     // return
    TK_IF,         // if
    TK_ELSE,       // else
    TK_WHILE,      // while
    TK_FOR,        // for
    TK_IDENT,      // 変数
    TK_NUM,        // 整数
    TK_TYPE,       // 型
    TK_EOF         // EOF
} TokenKind;

/* トークン */
typedef struct Token Token;
struct Token {
    TokenKind kind;   // トークンの種類
    const char *str;  // トークン文字列
    int len;          // トークンの長さ
    int num;          // 数値: kind が TK_NUM の場合
    Type type;        // 型: kind が TK_TYPE の場合
    Token *next;      // リスト
};

/* 抽象構文機のノードの種類 */
typedef enum {
    ND_NULL,     // NULL
    ND_ADD,      // +
    ND_SUB,      // -
    ND_MUL,      // *
    ND_DIV,      // /
    ND_EQ,       // ==
    ND_NE,       // !=
    ND_LT,       // <
    ND_LE,       // <=
    ND_NUM,      // 整数
    ND_ASSIGN,   // 代入
    ND_RETURN,   // return
    ND_IF,       // if-else
    ND_WHILE,    // while
    ND_FOR,      // for
    ND_LVAR,     // 変数
    ND_FUNC,     // 関数
    ND_DEFFUNC,  // 関数定義
    ND_BLOCK,    // ブロック
    ND_ADDR,     // アドレス取得
    ND_DEREF     // 参照外し
} NodeKind;

typedef struct LVar LVar;

/* 抽象構文機のノード */
typedef struct Node Node;
struct Node {
    NodeKind kind;  // ノードの種類
    union {
        // 数値
        struct {
            int val;
        } num;
        // 変数
        struct {
            char *name;
            int len;
            int offset;  // ベースポインタからのオフセット
        } lvar;
        // 関数
        struct {
            char *name;
            int len;
            Node **params;
            int max_param;  // パラメータノードを格納できる最大数
            int num_param;  // パラメータ数
        } func;
        // 関数定義
        struct {
            char *name;
            int len;
            Type rettype;
            LVar **params;
            int max_param;  // パラメータノードを格納できる最大数
            int num_param;  // パラメータ数
            Node *block;
        } deffunc;
        // 1項演算子
        struct {
            Node *expr;
        } op1;
        // 2項演算子
        struct {
            Node *lhs;  // 左辺
            Node *rhs;  // 右辺
        } op2;
        // if (test) tbody else ebody
        struct {
            Node *test;
            Node *tbody;  // then-body
            Node *ebody;  // else-body
        } cif;
        // while (test) body
        struct {
            Node *test;
            Node *body;
        } cwhile;
        // for (init, test, update) body
        struct {
            Node *init;
            Node *test;
            Node *update;
            Node *body;
        } cfor;
        // ブロック
        struct {
            Node **code;
            int max_code;     // コードを格納できる最大数
            int num_code;     // コード数
            LVar *locals;     // ローカル変数
            int num_local;    // ローカル変数の個数
            int total_local;  // このブロック以下の全てのブロックのローカル変数の個数
            Node *pblock;     // 親ブロック
        } block;
    } v;
};

/* ローカル変数の型 */
struct LVar {
    char *name;  // 変数の名前
    int len;     // 名前の長さ
    Type type;   // 型
    int offset;  // RBPからのオフセット
    LVar *next;  // 次の変数かNULL
};

/* 入力プログラム */
extern const char *user_input;

/* ブロック内の stmt 数 */
#define MAX_CODE (10)

/* 関数呼び出し時のパラメータ数 */
#define MAX_PARAM (6)

/* エラー出力関数 */
void error(char *fmt, ...);

/* エラー箇所を報告する */
void error_at(const char *exp, char *fmt, ...);

/* トークナイズする */
void tokenize(char *exp);

/* トークンが指定の記号なら true を返し、トークンを進める。
   別の記号なら false を返す。
 */
bool consume(const char *op);

/* トークンが指定の種類ならそのトークンを返し、トークンを進める。
   違っていたら NULL を返す。
*/
Token *consume_with_kind(TokenKind kind);

/* トークンが指定の記号なら、トークンを進める。
   違っていたらパニックする。
*/
void expect(const char *op);

/* トークンが kind ならそれを返し、トークンを進める。
   違っていたらパニックする。
 */
Token *expect_with_kind(TokenKind kind);

/* トークンを 1 つだけ先読みする。
 */
Token *peek(void);

/* EOFか？ */
bool eof(void);

/* パース */
Node *parse(void);

/* 抽象構文木を下りながらコードを生成 */
void gen(Node *node);
