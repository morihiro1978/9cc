#!/bin/bash

try() {
    expected="$1"
    input="$2"

    ./9cc "$input" > tmp.s
    gcc -o tmp tmp.s
    ./tmp
    actual="$?"

    if [ "$actual" = "$expected" ]; then
        echo "$input => $actual"
    else
        echo "$input => $expected expected, but got $actual"
        exit 1
    fi
}

try 0 "0;"
try 42 "42;"
try 21 "5+20-4;"
try 0 "5+20-4-21;"
try 0 "5+ 20 -4 - 21;"
try 1 "(1);"
try 9 "(10-5)+4;"
try 1 "10-(5+4);"
try 9 "(10-5+4);"
try 9 "((10-5)+4);"
try 6 "2 * 3;"
try 7 "1 + 2 * 3;"
try 9 "(1 + 2) * 3;"
try 10 "8+4 / 2;"
try 6 "(8 + 4) / 2;"
try 10 "-10+20;"
try 6 "-2*-3;"
try 1 "1==1;"
try 0 "1!=1;"
try 0 "1>2;"
try 0 "1>=2;"
try 1 "2>=2;"
try 0 "2<1;"
try 0 "2<=1;"
try 1 "2<=2;"
try 0 "1+1==1;"
try 2 "1+(1==1);"
try 1 "a=1;"
try 2 "b=2;"
try 3 "c=3;"
try 4 "d=4;"
try 5 "e=5;"
try 6 "f=6;"
try 7 "g=7;"
try 8 "h=8;"
try 9 "i=9;"
try 10 "j=10;"
try 11 "k=10+1;"
try 12 "l=10+2;"
try 13 "m=10+3;"
try 14 "n=10+4;"
try 15 "o=10+5;"
try 16 "p=10+6;"
try 17 "q=10+7;"
try 18 "r=10+8;"
try 19 "s=10+9;"
try 20 "t=2*10+0;"
try 21 "u=2*10+1;"
try 22 "v=2*10+2;"
try 23 "w=2*10+3;"
try 24 "x=2*10+4;"
try 25 "y=2*10+5;"
try 26 "z=2*10+6;"
try 0 "a=b=c=d=e=f=g=h=i=j=k=l=m=n=o=p=q=r=s=t=u=v=w=x=y=z=0;"
try 3 "a=1; b=a+2;"
try 18 "a = b = 3; c = a * a + b * b; c;"
try 6 "foo = 1; bar = 2 + 3; ans = foo + bar;"
try 1 "return 1;"
try 2 "return 2; return 1;"
try 6 "foo = 1; bar = 2 + 3; return foo + bar;"

echo OK
