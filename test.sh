#!/bin/bash

assert_char() {
  ascii_of "$1" ascii_num
  assert "$ascii_num" "$2" "" "$1"
}

assert_func() {
  func="foo"
  echo "
#include <stdio.h>
#include <stdlib.h>
int foo() {
  printf(\"Hello, world!\n\");
  return 1;
}

int bar(int a, int b) {
  printf(\"called bar: arg1:%d, arg2:%d\n\", a, b);
  return a+b;
}

void alloc(int **p, int a, int b, int c, int d) {
  *p = calloc(1, sizeof(int));
  **p = a; *(*p+1) = b;*(*p+2) = c;*(*p+3) = d;
}
  " > $func.c
  cc -g -c -o $func.o $func.c

  assert "$1" "$2" "$func.o"
  rm $func.c $func.o
}
  
assert() {
  expected="$1"
  input="$2"
  func_name="$3"
  char="$4"
  echo "$input" > tmp.c
  ./mycc tmp.c > tmp.s
  if [ $func_name ]; then
    cc -g -static -o tmp tmp.s $func_name
  else
    cc -g -static -o tmp tmp.s
  fi
  ./tmp
  actual="$?"

  if [ "$actual" = "$expected" ]; then
    if [ $char ]; then
      expected="$expected('$char')"
    fi

    echo "$input => $expected"
  else
    if [ $char != "" ]; then
      expected="$expected('$char')"
    fi

    echo "$input => $expected expected, but got $actual"
    exit 1
  fi
}

ascii_of() {
  char="$1"
  ascii_num=$(printf %d "'$char")
}

assert 0 " int main() { 0; }"
assert 42 " int main() { 42; }"
assert 21 " int main() { 5+20-4; }"
assert 21 " int main() { 5 + 20 - 4; }"
assert 47 " int main() { 5+6*7; }"
assert 15 " int main() { 5*(9-6); }"
assert 4 " int main() { (3+5)/2; }"
assert 5 " int main() { +5; }"
assert 13 " int main() { -5+10+8; }"
assert 1 " int main() { 10>=0; }"
assert 0 " int main() { 10<=5; }"
assert 1 " int main() { 10==10; }"
assert 0 " int main() { 10==1; }"
assert 1 " int main() { 10!=1; }"
assert 0 " int main() { 10!=10; }"
assert 0 " int main() { int a; a=0; }"
assert 1 " int main() { int a;int b;a=b=1; }"
assert 2 " int main() { int a;int b;a=1;b=1;a+b; }"
assert 2 " int main() { int a;int ab;a=1;ab=1;a+ab; }"
assert 4 " int main() { int a;int ab;a=1;ab=1;a+3; }"
assert 2 " int main() { int a;int ab;a=1;ab=1;return a+ab;a+3; }"
assert 1 " int main() { if(3 >= 0)  return 1; else  return 2; }"
assert 2 " int main() { if(3 <= 0)  return 1; else  return 2; }"
assert 10 " int main() { int i;i=0;while(i < 10)  i=i+1;return i; }"
assert 10 " int main() { int i;int j;j=0; for(i=0; i<10; i=i+1) j=j+1; return j; }"
assert 10 " int main() { 10; }"
assert 30 " int main() { int i;int j;int k;j=0;k=0; for(i=0; i<10; i=i+1){ j=j+1;k=k+2;} return j+k; }"
assert_func 1 "int main() {foo();}"
assert_func 3 "int main() {bar(1, 2);}"
assert_func 2 "int main() {int a;int b;a=0;b=2; bar(a, b);}"
assert 10 "int main() { foo();} int foo() {return 10;}"
assert 3 "int main() { int a;a=2;return foo(1, a);} int foo(int b, int c) {return b+c;}"

assert 21 "
int main() {
  return fib(7);
}
int fib(int a) {
  if(a==0) return 1;
  if(a==1) return 1;
  int b;int c;
  b=a-1;
  c=a-2;
  return fib(b) + fib(c);
}
"
assert 1 "int main() {int a;int *b;a=1;b=&a;return *b;}"
assert 127 "int main() { int a;a=2;return foo(1, a);} int foo(int b, int c) {return a+b+c;}"
assert 127 "int main() { {int a;}a=2;return a;}"
assert 1 "int main() {int a; a=1; int *b; b=&a; return *b;}"
assert 1 "int main() {int *a;int b; a=&b;*a=1; return b;}"
assert 21 "int main() {return foo(1,2,3,4,5,6);} int foo(int a, int b, int c, int d, int e, int f) {return a+b+c+d+e+f;}"
assert_func 3 "int main() {int *p;alloc(&p, 1, 2, 3, 4);int *q;q=p+2;return *q;}"
assert_func 4 "int main() {int *p;alloc(&p, 1, 2, 3, 4);int *q;q=p-2+5;return *q;}"
assert 4 "int main() {int a;return sizeof(a);}"
assert 4 "int main() {return sizeof(1);}"
assert 8 "int main() {int *a;return sizeof(a);}"
assert 8 "int main() {int *a;return sizeof(a+2);}"
assert 4 "int main() {int *a;return sizeof(sizeof(a));}"
assert_func 8 "int main() {int *p;alloc(&p, 1, 2, 3, 4);*(p+1)=8;return *(p+1);}"
assert 0 "int main() {int a[5];return 0;}"
assert 2 "int main() {int a[5];*a=1;*(a+1)=2;return *(a+1);}"
assert 1 "int main() {int a[5]; a[2] = 1;return a[2];}"
assert 1 "int a; int main() {a=1;return a;}"
assert 3 "int main() {char x[3];x[0] = -1;x[1] = 2;int y;y = 4;return x[0] + y;}"
assert_char "e" "int main() {char *s; s = \"Hello, world!\"; return s[1];}"

echo OK

