#!/bin/bash

try() {
    expected="$1"
    input="$2"

    gcc -c test.c
    ./9cc "$input" > app.s
    gcc -o app app.s test.o
    ./app
    actual="$?"

    if [ "$actual" = "$expected" ]; then
        echo "$input => $actual"
    else
        echo "$input => $expected expected, but got $actual"
        exit 1
    fi
}

try 0 "main(){ return 0; }"
try 42 "main(){ return 42; }"
try 21 "main(){ return 5+20-4; }"
try 0 "main(){ return 5+20-4-21; }"
try 0 "main(){ return 5+ 20 -4 - 21; }"
try 1 "main(){ return (1); }"
try 9 "main(){ return (10-5)+4; }"
try 1 "main(){ return 10-(5+4); }"
try 9 "main(){ return (10-5+4); }"
try 9 "main(){ return ((10-5)+4); }"
try 6 "main(){ return 2 * 3; }"
try 7 "main(){ return 1 + 2 * 3; }"
try 9 "main(){ return (1 + 2) * 3; }"
try 10 "main(){ return 8+4 / 2; }"
try 6 "main(){ return (8 + 4) / 2; }"
try 10 "main(){ return -10+20; }"
try 6 "main(){ return -2*-3; }"
try 1 "main(){ return 1==1; }"
try 0 "main(){ return 1!=1; }"
try 0 "main(){ return 1>2; }"
try 0 "main(){ return 1>=2; }"
try 1 "main(){ return 2>=2; }"
try 0 "main(){ return 2<1; }"
try 0 "main(){ return 2<=1; }"
try 1 "main(){ return 2<=2; }"
try 0 "main(){ return 1+1==1; }"
try 2 "main(){ return 1+(1==1); }"
try 1 "main(){ int a; return a=1; }"
try 2 "main(){ int b; return b=2; }"
try 3 "main(){ int c; return c=3; }"
try 4 "main(){ int d; return d=4; }"
try 5 "main(){ int e; return e=5; }"
try 6 "main(){ int f; return f=6; }"
try 7 "main(){ int g; return g=7; }"
try 8 "main(){ int h; return h=8; }"
try 9 "main(){ int i; return i=9; }"
try 10 "main(){ int j; return j=10; }"
try 11 "main(){ int k; return k=10+1; }"
try 12 "main(){ int l; return l=10+2; }"
try 13 "main(){ int m; return m=10+3; }"
try 14 "main(){ int n; return n=10+4; }"
try 15 "main(){ int o; return o=10+5; }"
try 16 "main(){ int p; return p=10+6; }"
try 17 "main(){ int q; return q=10+7; }"
try 18 "main(){ int r; return r=10+8; }"
try 19 "main(){ int s; return s=10+9; }"
try 20 "main(){ int t; return t=2*10+0; }"
try 21 "main(){ int u; return u=2*10+1; }"
try 22 "main(){ int v; return v=2*10+2; }"
try 23 "main(){ int w; return w=2*10+3; }"
try 24 "main(){ int x; return x=2*10+4; }"
try 25 "main(){ int y; return y=2*10+5; }"
try 26 "main(){ int z; return z=2*10+6; }"
try 0 "main(){ int a; int b; int c; int d; int e; int f; int g; int h; int i; int j; int k; int l; int m; int n; int o; int p; int q; int r; int s; int t; int u; int v; int w; int x; int y; int z; return a=b=c=d=e=f=g=h=i=j=k=l=m=n=o=p=q=r=s=t=u=v=w=x=y=z=0; }"
try 3 "main(){ int a; int b; a=1; return b=a+2; }"
try 18 "main(){ int a; int b; int c; a = b = 3; return c = a * a + b * b; return c; }"
try 6 "main(){ int foo; int bar; int ans; foo = 1; bar = 2 + 3; return ans = foo + bar; }"
try 1 "main(){ return 1; }"
try 2 "main(){ return 2; return 1; }"
try 6 "main(){ int foo; int bar; foo = 1; bar = 2 + 3; return foo + bar; }"
try 10 "main(){ if (1) return 10; return 20; }"
try 20 "main(){ if (0) return 10; return 20; }"
try 20 "main(){ if (1) if (0) return 10; return 20; }"
try 10 "main(){ if (1) if (1) return 10; return 20; }"
try 100 "main(){ int ans; int var; int expt; ans = 0; var = 10; expt = 10; if (var == expt) ans = 100; return ans; }"
try 0 "main(){ int ans; int var; int expt; ans = 0; var = 10; expt = 20; if (var == expt) ans = 100; return ans; }"
try 10 "main(){ int a; a = 0; if (1) a = 10; else a = 20; return a; }"
try 20 "main(){ int a; a = 0; if (0) a = 10; else a = 20; return a; }"
try 0 "main(){ int i; i = 10; while (i > 0) i = i - 1; return i; }"
try 10 "main(){ int a; int i; a=0; for(i=0; i<10; i=i+1) a=a+1; return a; }"
try 10 "main(){ int a; int i; a=0; i=0; for(; i<10; i=i+1) a=a+1; return a; }"
try 10 "main(){ int i; i=0; for(; i<10; ) i=i+1; return i; }"
try 10 "main(){ int a; a = 0; if (1){ a = 10; }else{ a = 20; } return a; }"
try 20 "main(){ int a; a = 0; if (0){ a = 10; }else{ a = 20; } return a; }"
try 0 "main(){ int i; i = 10; while (i > 0){ i = i - 1; } return i; }"
try 10 "main(){ int a; int i; a=0; for(i=0; i<10; i=i+1){ a=a+1; } return a; }"
try 10 "main(){ int a; int i; a=0; i=0; for(; i<10; i=i+1){ a=a+1; } return a; }"
try 10 "main(){ int i; i=0; for(; i<10; ){ i=i+1; } return i; }"
try 16 "main(){ int ans; int i; ans=1; for(i=0; i<4; ){ i=i+1; ans=ans*2; } return ans; }"
try 16 "main(){ int i; int ans; i=0; ans=1; while(i<4){ i=i+1; ans=ans*2; } return ans; }"
try 0 "main(){ func0(); return 0; }"
try 0 "main(){ int ret; ret=func0(); return ret; }"
try 1 "main(){ int ret; ret=func1(1); return ret; }"
try 3 "main(){ int ret; ret=func2(1,2); return ret; }"
try 6 "main(){ int ret; ret=func3(1,2,3); return ret; }"
try 10 "main(){ int ret; ret=func4(1,2,3,4); return ret; }"
try 15 "main(){ int ret; ret=func5(1,2,3,4,5); return ret; }"
try 21 "main(){ int ret; ret=func6(1,2,3,4,5,6); return ret; }"
try 1 "main(){ int p1; int ret; p1=1; ret=func1(p1); return ret; }"
try 3 "main(){ int p1; int p2; int ret; p1=1; p2=2; ret=func2(p1,p2); return ret; }"
try 6 "main(){ int p1; int p2; int p3; int ret; p1=1; p2=2; p3=3; ret=func3(p1,p2,p3); return ret; }"
try 10 "main(){ int p1; int p2; int p3; int p4; int ret; p1=1; p2=2; p3=3; p4=4; ret=func4(p1,p2,p3,p4); return ret; }"
try 15 "main(){ int p1; int p2; int p3; int p4; int p5; int ret; p1=1; p2=2; p3=3; p4=4; p5=5; ret=func5(p1,p2,p3,p4,p5); return ret; }"
try 21 "main(){ int p1; int p2; int p3; int p4; int p5; int p6; int ret; p1=1; p2=2; p3=3; p4=4; p5=5; p6=6; ret=func6(p1,p2,p3,p4,p5,p6); return ret; }"
try 1 "main(){ int a; a=0; {a=1;} return a; }"
try 1 "main(){ int a; {a=0; {a=1;} return a;} }"
try 1 "main(){ int a; {a=0; {{a=1;}} return a;} }"
try 1 "main(){ int a; {a=0; {{a=1;} return a;}} }"
try 1 "main(){ int a; a=0; {int b; b=1; a=b; } return a; }"
try 10 "f(){ return func1(10); } main(){ return f(); }"
try 10 "f(){ return func1(1); } main(){ int a; int i; a=0; for(i=0;i<10;i=i+1){ a=a+f(); func1(a); } return a; }"
try 123 "f(i){ return i; } main(){ return f(123); }"
try 0 "fib(n){ if(n==0){return 0;} if(n==1){return 1;} return fib(n-1) + fib(n-2); } main(){ fib(0); }"
try 1 "fib(n){ if(n==0){return 0;} if(n==1){return 1;} return fib(n-1) + fib(n-2); } main(){ fib(1); }"
try 1 "fib(n){ if(n==0){return 0;} if(n==1){return 1;} return fib(n-1) + fib(n-2); } main(){ fib(2); }"
try 2 "fib(n){ if(n==0){return 0;} if(n==1){return 1;} return fib(n-1) + fib(n-2); } main(){ fib(3); }"
try 3 "fib(n){ if(n==0){return 0;} if(n==1){return 1;} return fib(n-1) + fib(n-2); } main(){ fib(4); }"
try 5 "fib(n){ if(n==0){return 0;} if(n==1){return 1;} return fib(n-1) + fib(n-2); } main(){ fib(5); }"
try 8 "fib(n){ if(n==0){return 0;} if(n==1){return 1;} return fib(n-1) + fib(n-2); } main(){ fib(6); }"
try 13 "fib(n){ if(n==0){return 0;} if(n==1){return 1;} return fib(n-1) + fib(n-2); } main(){ fib(7); }"
try 21 "fib(n){ if(n==0){return 0;} if(n==1){return 1;} return fib(n-1) + fib(n-2); } main(){ fib(8); }"
try 34 "fib(n){ if(n==0){return 0;} if(n==1){return 1;} return fib(n-1) + fib(n-2); } main(){ fib(9); }"
try 55 "fib(n){ if(n==0){return 0;} if(n==1){return 1;} return fib(n-1) + fib(n-2); } main(){ fib(10); }"
try 0 "func_a(){ func0(); } main(){ func_a(); return 0; }"
try 10 "f(){ return func1(1); } main(){a=0; for(_=0;_<10;_=_+1){ a=a+f(); func1(a); } return a;}"
try 123 "main(){ int x; int y; int z; x=123; y=&x; z=func1(*y); return z; }"

echo OK
