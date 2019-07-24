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

try 0 0
try 42 42
try 21 "5+20-4"
try 0 "5+20-4-21"
try 0 "5+ 20 -4 - 21"
try 1 "(1)"
try 9 "(10-5)+4"
try 1 "10-(5+4)"
try 9 "(10-5+4)"
try 9 "((10-5)+4)"

echo OK
