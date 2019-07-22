#include <stdio.h>
#include <stdlib.h>

int main(int argc, char **argv) {
    if (argc != 2) {
        fprintf(stderr, "引数の個数が間違っています。\n");
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
            fprintf(stderr, "不正な式です。\n");
            return 1;
        }
    }
    printf("    ret\n");

    return 0;
}
