#!/bin/bash
assert() {
  expected="$1"
  input="$2"
  
  ./mycc "$input" > tmp.s
  cc -o tmp tmp.s
  ./tmp
  actual="$?"

  if [ "$actual" = "$expected" ]; then
    echo "$input => $actual"
  else
    echo "$input => $expected expected, but got $actual"
    exit 1
  fi
}

assert 0 0
assert 42 42
assert 21 "5+20-4"
assert 21 "5 + 20 - 4"
assert 47 "5+6*7"
assert 15 "5*(9-6)"
assert 4 "(3+5)/2"
assert 5 "+5"
assert 13 "-5+10+8"
assert 1 "10>=0"
assert 0 "10<=5"
assert 1 "10==10"
assert 0 "10==1"
assert 1 "10!=1"
assert 0 "10!=10"

echo OK
