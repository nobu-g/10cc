#!/bin/bash

assert() {
  expected="$1"
  input="$2"

  ./9cc "$input" >tmp.s
  cc -o tmp tmp.s
  ./tmp
  actual="$?"

  if [[ "$actual" = "$expected" ]]; then
    echo "$input => $actual"
  else
    echo "$input => $expected expected, but got $actual"
  fi
}

assert 0 "0;"
assert 42 "42;"
assert 42 "21+63-42;"
assert 42 "21 +  63 -   42;"
assert 47 "5+6*7;"
assert 15 "5*(9-6);"
assert 4 "(3+5)/2;"
assert 4 "-(3+5)/-(2);"
assert 2 "-3+5/2 + 3;"
assert 1 "1 == 1;"
assert 1 " (1 != 3) * 4 +  (4 - 1) == 7; "
assert 0 "(1 <= ((2*4) - 1)) - (1 > 0) * 1 * 1;"
assert 0 "(1 < (5 + -10) * 2) == (10 < (120 - 60));"
assert 1 "a = 1;"
assert 8 "a=1; b=1; c = a + b; a = b + c; b = c + a; a + b;"
assert 6 "foo = 1; bar = 2 + 3; return foo + bar;"
assert 42 "return 42; return 420;"
assert 42 "if (1) return 42;"
assert 42 "if (1) return 42; else return 64;"
assert 64 "if (0) return 42; else return 64;"
assert  5 "num=10;if((num/3)*3 == num)return 3;else if((num/5)*5 == num)return 5;else return 0;"
assert 10 "a = 0; while (a < 10) a = a + 1;"
assert 55 "total = 0; for (i=1; i <= 10; i=i+1) total = total + i; return total;"

echo OK
