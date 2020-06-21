#!/bin/bash

set -u

assert() {
  expected="$1"
  input="$2"

  ./10cc "$input" >tmp.s
  cc -o tmp tmp.s
  ./tmp
  actual="$?"

  if [[ "$actual" = "$expected" ]]; then
    echo "[PASSED] $input => $actual"
    return 0
  else
    echo "[FAILED] $input => $expected expected, but got $actual"
    return 1
  fi
}

assert 42 "main() {return 42;}"
assert 2 "main() {return 1 + 1;}"
assert 4 "main() {return (1 + 1) * 2;}"
assert 1 "main() {return (1 + 1) / 2;}"
assert 4 "main() {a=1; b=1; c = a + b; return a + b + c;}"
assert 2 "foo() {return 2;} main() {return foo();}"
assert 3 "foo(a, b) {return a + b;} main() {return 0 + foo(1, 2);}"
assert 42 "main() {if (1) return 42;}"
assert 42 "main() {if (1) return 42; else return 64;}"
assert 64 "main() {if (0) return 42; else return 64;}"
assert 5 "main() {num=10;if((num/3)*3 == num)return 3;else if((num/5)*5 == num)return 5;else return 0;}"
assert 10 "main() {a = 0; while (a < 10) a = a + 1; return a;}"
assert 55 "main() {total = 0; for (i=1; i <= 10; i=i+1) total = total + i; return total;}"
assert 110 "main() {total = 0; for (i=1; i <= 10; i=i+1) {total = total + i; total = total + i;} return total;}"
assert 13 "fibo(n) {if (n < 2) return 1; else return fibo(n-2) + fibo(n-1);} main() {return fibo(6);}"
assert 3 "main() {x = 3; y = &x; return *y;}"
