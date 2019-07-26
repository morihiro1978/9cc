#include "9cc.h"

/* 入力プログラム */
const char *user_input = NULL;

/* エラー出力関数 */
void error(char *fmt, ...) {
    va_list ap;
    va_start(ap, fmt);
    vfprintf(stderr, fmt, ap);
    fprintf(stderr, "\n");
    exit(1);
}

/* エラー箇所を報告する */
void error_at(const char *exp, char *fmt, ...) {
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