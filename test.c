#include <stdio.h>

int func0(void) {
    printf("func0\n");
    return 0;
}

int func1(int p1) {
    printf("func1: p1:%d\n", p1);
    return p1;
}

int func2(int p1, int p2) {
    printf("func2: p1:%d, p2:%d\n", p1, p2);
    return p1 + p2;
}

int func3(int p1, int p2, int p3) {
    printf("func3: p1:%d, p2:%d, p3:%d\n", p1, p2, p3);
    return p1 + p2 + p3;
}

int func4(int p1, int p2, int p3, int p4) {
    printf("func4: p1:%d, p2:%d, p3:%d, p4:%d\n", p1, p2, p3, p4);
    return p1 + p2 + p3 + p4;
}

int func5(int p1, int p2, int p3, int p4, int p5) {
    printf("func5: p1:%d, p2:%d, p3:%d, p4:%d, p5:%d\n", p1, p2, p3, p4, p5);
    return p1 + p2 + p3 + p4 + p5;
}

int func6(int p1, int p2, int p3, int p4, int p5, int p6) {
    printf("func6: p1:%d, p2:%d, p3:%d, p4:%d, p5:%d, p6:%d\n",
           p1,
           p2,
           p3,
           p4,
           p5,
           p6);
    return p1 + p2 + p3 + p4 + p5 + p6;
}