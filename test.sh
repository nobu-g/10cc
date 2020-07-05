#!/bin/bash

set -u

assert() {
  expected="$1"
  input="$2"

  ./10cc "$input" > tmp.s
  cc -c tmp.s    # -> tmp.o
  cc -c helper.c # -> helper.o
  cc -o tmp tmp.o helper.o
  # cc -o tmp tmp.s
  ./tmp
  actual="$?"

  if [[ "$actual" = "$expected" ]]; then
    echo -e "\033[32m[PASSED]\033[m $input => $actual"
    return 0
  else
    echo -e "\033[31m[FAILED]\033[m $input => $expected expected, but got $actual"
    return 1
  fi
}

assert 42 "int main() {return 42;}"
assert 2 "int main() {return 1 + 1;}"
assert 4 "int main() {return (1 + 1) * 2;}"
assert 1 "int main() {return (1 + 1) / 2;}"
assert 1 "int main() {int a; a = 1; return a;}"
assert 2 "int main() {int a; a = 1 + 1; return a;}"
assert 2 "int main() {int a; a = 1; a = a + 1; return a;}"
assert 2 "int main() {int a; int b; a=1; b=1; return a + b;}"
assert 3 "int main() {int a; a = 1; return a + a + a;}"
assert 4 "int main() {int a; int b; int c; a=1; b=1; c = a + b; return a + b + c;}"
assert 2 "int foo() {return 2;} int main() {return foo();}"
assert 3 "int foo(int a, int b) {return a + b;} int main() {return 0 + foo(1, 2);}"
assert 42 "int main() {if (1) return 42;}"
assert 42 "int main() {if (1) return 42; else return 64;}"
assert 64 "int main() {if (0) return 42; else return 64;}"
assert 5 "int main() {int num; num=10;if((num/3)*3 == num)return 3;else if((num/5)*5 == num)return 5;else return 0;}"
assert 10 "int main() {int a; a = 0; while (a < 10) a = a + 1; return a;}"
assert 55 "int main() {int total; int i; total = 0; for (i=1; i <= 10; i=i+1) total = total + i; return total;}"
assert 110 "int main() {int total; int i; total = 0; for (i=1; i <= 10; i=i+1) {total = total + i; total = total + i;} return total;}"
assert 2 "int bar(int m) {return m+1;} int foo(int n) {return bar(n+1);} int main() {return foo(0);}"
assert 24 "int fact(int n) {if (n < 2) return 1; else return n * fact(n-1);} int main() {return fact(4);}"
assert 13 "int fibo(int n) {if (n < 2) return 1; else return fibo(n-2) + fibo(n-1);} int main() {return fibo(6);}"
assert 3 "int main() {int x; int *y; x = 3; y = &x; return *y;}"
assert 42 "int ***ptr() {int foo; int *var; return 42;} int main() {int *****a; return ptr();}"
assert 1 "int main() {int a; int b; int c; int d; a = 1; b = 2; c = 3; d = 4; int *p; p = &b; int *q; q = p + 2; return *q;}"
assert 2 "int main() {int a; int b; int c; int d; a = 1; b = 2; c = 3; d = 4; int *p; p = &c; int *q; q = p + 2; return *q;}"
assert 1 "int main() {int a; int b; int c; int d; a = 1; b = 2; c = 3; d = 4; int *p; p = &b + 2; return *p;}"
assert 2 "int main() {int a; int b; int c; int d; a = 1; b = 2; c = 3; d = 4; int *p; p = &c + 2; return *p;}"
assert 2 "int main() {int a; int b; int c; int d; a = 1; b = 2; c = 3; d = 4; int *p; p = &d + 4; return *p;}"
assert 4 "int main() {int *p; alloc4(&p, 1, 2, 4, 8); int *q; q = p + 2; return *q;}"
assert 4 "int main() {return sizeof(4);}"
assert 8 "int main() {int *a; return sizeof(a);}"
assert 8 "int main() {int a; a = 42; return sizeof(&a);}"
assert 0 "int main() {int a[10]; return 0;}"
assert 40 "int main() {int a[10]; return sizeof(a);}"
assert 1 "int main() {int b; int *a; a = &b; *a = 1; return 1;}"
assert 4 "int main() {int a[10]; *a = 4; return 4;}"
assert 3 "int main() {int a[2]; *a = 1; *(a + 1) = 2; int *p; p = a; return *p + *(p + 1);}"
