#!/usr/bin/env bash

declare -a inputs
declare -a outputs

append() {
  inputs+=( "$1" )
  outputs+=( "$2" )
}

append "return 42;" 42
append "return 1 + 1;" 2
append "return (1 + 1) * 2;" 4
append "return (1 + 1) / 2;" 1
append "return (1 != 3) * 4 +  (4 - 1) == 7;" 1
append " 5;;; return (1 + 6) / 2;" 3
append "int a; a = 1; return a;" 1
append "int a; a = 1 + 1; return a;" 2
append "int a; a = 1; a = a + 1; return a;" 2
append "int a; int b; a=1; b=1; return a + b;" 2
append "int a; a = 1; return a + a + a;" 3
append "int a; int b; int c; a=1; b=1; c = a + b; return a + b + c;" 4
append "return foo();" 3
append "return add(1, 2);" 3
append "if (1) return 42;" 42
append "if (1) return 42; else return 64;" 42
append "if (0) return 42; else return 64;" 64
append "if (1) ; else{} return 8;" 8
append "int num; num=10;if((num/3)*3 == num)return 3;else if((num/5)*5 == num)return 5;else return 0;" 5
append "int a; a = 0; while (a < 10) a = a + 1; return a;" 10
append "int total; int i; total = 0; for (i=1; i <= 10; i=i+1) total = total + i; return total;" 55
append "int total; int i; total = 0; for (i=1; i <= 10; i=i+1) {total = total + i; total = total + i;} return total;" 110
append "int i; i = 0; for (; i < 10;) {i = i + 1;} return i;" 10
append "int i; i = 0; for (;;) {i = i + 1; if (i >= 5) return i;} return i;" 5
append "return f1(0);" 2
append "return fact(4);" 24
append "return fibo(6);" 13
append "int x; int *p; x = 3; p = &x; return *p;" 3
append "int *****a; return 42;" 42
append "int a; int b; int c; int d; a = 1; b = 2; c = 3; d = 4; int *p; p = &c; int *q; q = p + 2; return *q;" 1
append "int a; int b; int c; int d; a = 1; b = 2; c = 3; d = 4; int *p; p = &d + 2; return *p;" 2
append "return sizeof(4);" 4
append "int *a; return sizeof(a);" 8
append "int a; a = 42; return sizeof(&a);" 8
append "int a[3]; return sizeof(*a);" 4
append "char c; return sizeof(c);" 1
append "char c; return sizeof(c + c);" 4
append "char c; int i; return sizeof(c = i);" 1
append "char c; int i; return sizeof(i = c);" 4
append "int a[10]; return sizeof(a);" 40
append "return sizeof(sizeof(sizeof(0)));" 4
append "return sizeof sizeof 4;" 4
append "return sizeof(int);" 4
append "return sizeof(char ***);" 8
append "int *a; int b; a = &b; *a = 4; return 1;" 1
append "int x; int *y; y = &x; *y = 3; return x;" 3
append "int x; x = -3; int *p; p = &x; return *p+-*p*2;" 3
append "int a; int b; a = b = 3; return a + b;" 6
append "int a; int b; b = 4; b = b + (a = 3); return b;" 7
append "int a[1]; *a = 4; return *a;" 4
append "int a[2]; *a = 1; *(a + 1) = 5; return *a + *(a + 1);" 6
append "int a[2]; *a = 1; *(a + 1) = 2; int *p; p = a; return *p + *(p + 1);" 3
append "int a[2]; *a = 1; *(a + 1) = 2; int *p; p = a; return *(p + 1);" 2
append "int a[2]; *a = 1; *(a + 1) = 2; return *(a + 1);" 2
append "int a[2]; *a = 1; *(a + 1) = 2; return a[0] + a[1];" 3
append "int a[2]; *a = 1; *(a + 1) = 2; return 0[a] + 1[a];" 3
append "gvar = 34; return gvar;" 34
append "int G; G = 34; pg = &G; return *pg;" 34
append "garr[1] = 2; return garr[1];" 2
append "char a; return sizeof(a);" 1
append "char a; a = 3; return 3;" 3
append "char x[3]; x[0] = -1; x[1] = 2; int y; y = 4; return x[0] + y;" 3
append "char a; char b; char c; char d; a = 1; b = 2; c = 3; d = 4; char *p; p = &c; char *q; q = p + 2; return *q;" 1
append "int a; int b; int c; int d; a = 1; b = 2; c = 3; d = 4; int *p; p = &c; int *q; q = p - 1; return *q;" 4
append "char a; char b; char c; char d; a = 1; b = 2; c = 3; d = 4; char *p; p = &d + 2; return *p;" 2
append "char arr[4]; return &arr[3] - &arr[0];" 3
append "int arr[4]; return &arr[3] - &arr[0];" 3
append "int a = 1; int b = 2; return sizeof(&a - &b);" 4  # FIXME: 8
append "int a; a = 1; char b; b = 9; if (1) {int b; b = 32; a = 100; } if(1) {char b; b = 5;} return a + b;" 109
append "int x[2][3]; int *y; y=x[0]; y[0]=0; return x[0][0];" 0
append "int x[2][3]; int *y; y=x[0]; y[1]=1; return x[0][1];" 1
append "int x[2][3]; int *y; y=x[0]; y[2]=2; return x[0][2];" 2
append "int x[2][3]; int *y; y=x[1]; y[0]=3; return x[1][0];" 3
append "int x[2][3]; int *y; y=x[1]; y[1]=4; return x[1][1];" 4
append "int x[2][3]; int *y; y=x[1]; y[2]=5; return x[1][2];" 5
append "int a[2][3]; return sizeof(*a);" 12
append "return 1 /*+ 1*/;" 1
append "char* foo; foo = \"bar\"; return 0;" 0
append "return first(\"bcd\") - first(\"abc\");" 1
append "return ({ 0; });" 0
append "return ({ 0; 1; 2; });" 2
append "({ 0; return 1; 2; }); return 3;" 1
append "return ({ 1; }) + ({ 2; }) + ({ 3; });" 6
append "return ({ int x; x=3; x; });" 3
append "return ({ int a; a=3; a = a + 3; });" 6
append "return ({ int a; a = 0; while (a < 10) a = a + 1; a; });" 10
append "return ({ int total; int i; total = 0; for (i=1; i <= 10; i=i+1) total = total + i; total; });" 55
append "return add(1, ({ int a[3] = {0, 1, 2}; a[1]; }));" 2
append "return add(0, ({ int i = 0; for (;i < 10;) {i = i + 1; } i; }));" 10
append "int x = 3; return x;" 3
append "int x = foo(); return x;" 3
append "char x = 3; return x;" 3
append "int x = {3}; return x;" 3
append "int a[3] = {0, 1, 2}; return a[0];" 0
append "int a[3] = {0, 1, 2}; return a[1];" 1
append "int a[3] = {0, 1, 2}; return a[2];" 2
append "int a[3] = {3}; return a[0];" 3
append "int a[3] = {3}; return a[1];" 0
append "int a[3] = {3}; return a[2];" 0
append "int a[] = {0, 1, 2}; return a[0];" 0
append "int a[] = {0, 1, 2}; return a[1];" 1
append "int a[] = {0, 1, 2}; return a[2];" 2
append "int a[2][3] = {{0, 1, 2}, {3, 4, 5}}; return a[1][1];" 4
append "int a[][3] = {{0, 1, 2}, {3, 4, 5}}; return a[1][0];" 3
append "int a[][3] = {{1, 2}, {3}}; return a[0][0];" 1
append "int a[][3] = {{1, 2}, {3}}; return a[0][1];" 2
append "int a[][3] = {{1, 2}, {3}}; return a[0][2];" 0
append "int a[][3] = {{1, 2}, {3}}; return a[1][0];" 3
append "int a[][3] = {{1, 2}, {3}}; return a[1][1];" 0
append "int a[][3] = {{1, 2}, {3}}; return a[1][2];" 0
append "int a[][2][3] = {{{}, {1, 2, 3}}, {{}, {}}}; return a[0][1][2];" 3
append "char a[4] = \"ABC\"; return a[0];" 65
append "char a[4] = \"ABC\"; return a[1];" 66
append "char a[4] = \"ABC\"; return a[2];" 67
append "char a[4] = \"ABC\"; return a[3];" 0
append "char str[] = \"hello\"; return sizeof(str);" 6
append "char c = 3; return add(c, c + 1);" 7
append "return add(0, first(\"ABC\"));" 65
append "int x = 2; x++; return x;" 3
append "int x = 2; x--; return x;" 1
append "int x = 2; return x++;" 2
append "int x = 2; return x--;" 2
append "int x = 2; return ++x;" 3
append "int x = 2; return --x;" 1
append "int x = 2; return x++ + ++x;" 6
append "int x = 2; int *p = &x; ++*p; return x;" 3
append "struct {int a; char b;} X; return sizeof(X);" 5
append "struct {int a; char b;} X[3]; return sizeof(X);" 15
append "struct {int a; char b;} X; X.a = 1; X.b = 2; return X.a;" 1
append "struct {int a; char b;} X; X.a = 1; X.b = 2; return X.b;" 2
append "struct X {int a; char b;}; struct X x; x.a = 1; x.b = 2; return x.a;" 1
append "struct X {int a; char b;}; struct X x; x.a = 1; x.b = 2; return x.b;" 2
append "struct Vector {int x; int y;}; struct Vector a; struct Vector *pa = &a; pa->x = 1; pa->y = 2; return pa->x;" 1
append "struct Vector {int x; int y;}; struct Vector a; struct Vector *pa = &a; pa->x = 1; pa->y = 2; return pa->y;" 2
append "struct Vector {int x; int y;}; struct Vector a; struct Vector *pa = &a; pa->x = 1; return a.x;" 1
append "int a = 0; a += 1; return a;" 1
append "int a = 2; a -= 1; return a;" 1
append "int a = 0; return a += 1;" 1
append "int a = 2; return a -= 1;" 1
append "int arr[] = {0, 1, 2}; int *p = arr; p += 1; return *p;" 1
append "int arr[] = {0, 1, 2}; int *p = arr; p += 2; return *p;" 2
append "int a = 14; a *= 3; return a;" 42
append "int a = 42; a /= 3; return a;" 14
append "return ginit;" 3
append "return giarr[0];" 1
append "return giarr[1];" 2
append "return giarr[2];" 3
append "return str[0];" 65
append "return str[1];" 66
append "return str[2];" 67
append "return str[3];" 0
append "return strarr[0][0];" 115
append "return sizeof(arrarr);" 16
append "return arrarr[0][0];" 1
append "return arrarr[0][1];" 2
append "return arrarr[1][0];" 3
append "return arrarr[1][1];" 0
append "return include1;" 11
append "return include2;" 22

cat <<EOF
// This code is generated by test/prep.sh

// null directive
#
/* */ #

// include directive
#include "include1.h"

int assert(int expected, int actual, char *code);

int foo() { return 3; }
int add(int a, int b) {return a + b;}
int f2(int m); int f1(int n) {return f2(n+1);}
int f2(int m) {return m+1;}
int fact(int m); int fact(int n) {if (n < 2) return 1; else return n * fact(n-1);}
int fibo(int n) { if (n == 1) return 1; else if (n == 0) return 1; else return fibo(n-1) + fibo(n-2); }
int gvar;
int *pg;
int garr[3];
char first(char *str) { return str[0]; }
int ginit = 3;
int giarr[] = {1, 2, 3};
char str[] = "ABC";
char *strarr[] = {"str0", "str1", "str2"};
int arrarr[2][2] = {{1, 2}, {3, 0}};

EOF

for i in "${!inputs[@]}"; do
  echo "int test${i}() { ${inputs[i]} }"
done

echo -e "\nint main() {"
for i in "${!inputs[@]}"; do
  echo "    assert(${outputs[i]}, test${i}(), \"{ $(echo -E "${inputs[i]}" | sed -e 's/\"/\\\"/g') }\");"
done
echo -e "\n    return 0;\n}"
