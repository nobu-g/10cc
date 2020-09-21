#!/usr/bin/env bash

set -u

assert() {
  expected="$1"
  input="$2"

  ./build/10cc "$input" > tests/tmp.s
  cc -c tests/tmp.s -o tests/tmp.o
  cc -c tests/helper.c -o tests/helper.o
  cc -static -o tests/tmp tests/tmp.o tests/helper.o
  ./tests/tmp
  actual="$?"

  if [[ "$actual" = "$expected" ]]; then
    echo -e "\e[32m[PASSED]\e[m $input \e[33m=> \e[36m$actual\e[m"
    return 0
  else
    echo -e "\e[31m[FAILED]\e[m $input \e[33m=> \e[36m$expected\e[m expected, but got \e[36m$actual\e[m"
    return 1
  fi
}

assert 42 "int main() {return 42;}"
assert 2 "int main() {return 1 + 1;}"
assert 4 "int main() {return (1 + 1) * 2;}"
assert 1 "int main() {return (1 + 1) / 2;}"
assert 1 "int main() {return (1 != 3) * 4 +  (4 - 1) == 7;}"
assert 3 "int main() { 5;;; return (1 + 6) / 2;}"
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
assert 8 "int main() {if (1) ; else{} return 8;}"
assert 5 "int main() {int num; num=10;if((num/3)*3 == num)return 3;else if((num/5)*5 == num)return 5;else return 0;}"
assert 10 "int main() {int a; a = 0; while (a < 10) a = a + 1; return a;}"
assert 55 "int main() {int total; int i; total = 0; for (i=1; i <= 10; i=i+1) total = total + i; return total;}"
assert 110 "int main() {int total; int i; total = 0; for (i=1; i <= 10; i=i+1) {total = total + i; total = total + i;} return total;}"
assert 2 "int bar(int m) {return m+1;} int foo(int n) {return bar(n+1);} int main() {return foo(0);}"
assert 24 "int fact(int n) {if (n < 2) return 1; else return n * fact(n-1);} int main() {return fact(4);}"
assert 13 "int fibo(int n) {if (n < 2) return 1; else return fibo(n-2) + fibo(n-1);} int main() {return fibo(6);}"
assert 3 "int main() {int x; int *p; x = 3; p = &x; return *p;}"
assert 42 "int ***ptr() {int foo; int *var; return 42;} int main() {int *****a; return ptr();}"
assert 1 "int main() {int a; int b; int c; int d; a = 1; b = 2; c = 3; d = 4; int *p; p = &c; int *q; q = p + 2; return *q;}"
assert 2 "int main() {int a; int b; int c; int d; a = 1; b = 2; c = 3; d = 4; int *p; p = &d + 2; return *p;}"
assert 4 "int main() {return sizeof(4);}"
assert 8 "int main() {int *a; return sizeof(a);}"
assert 8 "int main() {int a; a = 42; return sizeof(&a);}"
assert 4 "int main() {int a[3]; return sizeof(*a);}"
assert 1 "int main() {char c; return sizeof(c);}"
assert 4 "int main() {char c; return sizeof(c + c);}"
assert 1 "int main() {char c; int i; return sizeof(c = i);}"
assert 4 "int main() {char c; int i; return sizeof(i = c);}"
assert 40 "int main() {int a[10]; return sizeof(a);}"
assert 4 "int main() {return sizeof(sizeof(sizeof(0)));}"
assert 4 "int main() {return sizeof sizeof 4;}"
assert 4 "int main() {return sizeof(int);}"
assert 8 "int main() {return sizeof(char ***);}"
assert 1 "int main() {int *a; int b; a = &b; *a = 4; return 1;}"
assert 3 "int main() {int x; int *y; y = &x; *y = 3; return x;}"
assert 3 "int main() {int x; x = -3; int *p; p = &x; return *p+-*p*2;}"
assert 6 "int main() {int a; int b; a = b = 3; return a + b;}"
assert 7 "int main() {int a; int b; b = 4; b = b + (a = 3); return b;}"
assert 4 "int main() {int a[1]; *a = 4; return *a;}"
assert 6 "int main() {int a[2]; *a = 1; *(a + 1) = 5; return *a + *(a + 1);}"
assert 3 "int main() {int a[2]; *a = 1; *(a + 1) = 2; int *p; p = a; return *p + *(p + 1);}"
assert 2 "int main() {int a[2]; *a = 1; *(a + 1) = 2; int *p; p = a; return *(p + 1);}"
assert 2 "int main() {int a[2]; *a = 1; *(a + 1) = 2; return *(a + 1);}"
assert 3 "int main() {int a[2]; *a = 1; *(a + 1) = 2; return a[0] + a[1];}"
assert 3 "int main() {int a[2]; *a = 1; *(a + 1) = 2; return 0[a] + 1[a];}"
assert 34 "int g; int main() { g = 34; return g;}"
assert 34 "int *g; int main() { int G; G = 34; g = &G; return *g;}"
assert 2 "int g[3]; int main() {g[1] = 2; return g[1];}"
assert 1 "int main() {char a; return sizeof(a);}"
assert 3 "int main() {char a; a = 3; return 3;}"
assert 3 "int main() {char x[3]; x[0] = -1; x[1] = 2; int y; y = 4; return x[0] + y;}"
assert 1 "int main() {char a; char b; char c; char d; a = 1; b = 2; c = 3; d = 4; char *p; p = &c; char *q; q = p + 2; return *q;}"
assert 4 "int main() {int a; int b; int c; int d; a = 1; b = 2; c = 3; d = 4; int *p; p = &c; int *q; q = p - 1; return *q;}"
assert 2 "int main() {char a; char b; char c; char d; a = 1; b = 2; c = 3; d = 4; char *p; p = &d + 2; return *p;}"
assert 3 "int main() {char arr[4]; return &arr[3] - &arr[0];}"
assert 3 "int main() {int arr[4]; return &arr[3] - &arr[0];}"
assert 2 "int main() {int a; int a; a = 2; int a; return a;}"
