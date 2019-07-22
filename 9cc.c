#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>

void error(char *fmt, ...) {
    va_list ap;
    va_start(ap, fmt);
    vfprintf(stderr, fmt, ap);
    fprintf(stderr, "\n");
    exit(1);
}

int main(int argc, char **argv) {
    if (argc != 2) {
        error("引数の個数が間違っています。");
        return 1;
    }

    char *p = argv[1];

    printf(".intel_syntax noprefix\n");
    printf(".global main\n");
    printf("main:\n");
    printf("    mov rax, %ld\n", strtol(p, &p, 10));

    while (1) {
        if (p[0] == '+') {
            p = p + 1;
            printf("    add rax, %ld\n", strtol(p, &p, 10));
        } else if (p[0] == '-') {
            p = p + 1;
            printf("    sub rax, %ld\n", strtol(p, &p, 10));
        } else if (p[0] == '\0') {
            break;
        } else {
            error("不正な式です。\n");
            return 1;
        }
    }
    printf("    ret\n");

    return 0;
}
