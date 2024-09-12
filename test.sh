#!/bin/bash
assert_func() {
  func="foo"
  echo "
#include <stdio.h>
int foo() {
  printf(\"Hello, world!\n\");
  return 1;
}

int bar(int a, int b) {
  printf(\"called bar: arg1:%d, arg2:%d\n\", a, b);
  return a+b;
}
  " > $func.c
  cc -c -o $func.o $func.c

  assert "$1" "$2" "$func.o"
  rm $func.c $func.o
}
  
assert() {
  expected="$1"
  input="$2"
  ./mycc "$input" > tmp.s
  if [ "$3" != "" ]; then
    cc -o tmp tmp.s $3
  else
    cc -o tmp tmp.s
  fi
  ./tmp
  actual="$?"

  if [ "$actual" = "$expected" ]; then
    echo "$input => $actual"
  else
    echo "$input => $expected expected, but got $actual"
    exit 1
  fi
}

assert 0 "0;"
assert 42 "42;"
assert 21 "5+20-4;"
assert 21 "5 + 20 - 4;"
assert 47 "5+6*7;"
assert 15 "5*(9-6);"
assert 4 "(3+5)/2;"
assert 5 "+5;"
assert 13 "-5+10+8;"
assert 1 "10>=0;"
assert 0 "10<=5;"
assert 1 "10==10;"
assert 0 "10==1;"
assert 1 "10!=1;"
assert 0 "10!=10;"
assert 0 "a=0;"
assert 1 "a=b=1;"
assert 2 "a=1;b=1;a+b;"
assert 2 "a=1;ab=1;a+ab;"
assert 4 "a=1;ab=1;a+3;"
assert 2 "a=1;ab=1;return a+ab;a+3;"
assert 1 "if(3 >= 0)  return 1; else  return 2;"
assert 2 "if(3 <= 0)  return 1; else  return 2;"
assert 10 "i=0;while(i < 10)  i=i+1;return i;"
assert 10 "j=0; for(i=0; i<10; i=i+1) j=j+1; return j;"
assert 10 "{10;}"
assert 30 "j=0;k=0; for(i=0; i<10; i=i+1){ j=j+1;k=k+2;} return j+k;"
assert_func 1 "foo();"
assert_func 3 "bar(1, 2);"
assert_func 2 "a=0;b=2; bar(a, b);"

echo OK
