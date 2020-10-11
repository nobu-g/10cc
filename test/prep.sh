#!/usr/bin/env bash

declare -A tests=(

  ["return 42;"]="42"
  ["return 1 + 1;"]="2"
  ["return (1 + 1) * 2;"]="4"
  ["return (1 + 1) / 2;"]="1"
  ["return (1 != 3) * 4 +  (4 - 1) == 7;"]="1"
  [" 5;;; return (1 + 6) / 2;"]="3"
  ["int a; a = 1; return a;"]="1"
  ["int a; a = 1 + 1; return a;"]="2"
  ["int a; a = 1; a = a + 1; return a;"]="2"
  ["int a; int b; a=1; b=1; return a + b;"]="2"
  ["int a; a = 1; return a + a + a;"]="3"
  ["int a; int b; int c; a=1; b=1; c = a + b; return a + b + c;"]="4"
  ["return foo();"]="3"
  ["return add(1, 2);"]="3"
  ["if (1) return 42;"]="42"
  ["if (1) return 42; else return 64;"]="42"
  ["if (0) return 42; else return 64;"]="64"
  ["if (1) ; else{} return 8;"]="8"
  ["int num; num=10;if((num/3)*3 == num)return 3;else if((num/5)*5 == num)return 5;else return 0;"]="5"
  ["int a; a = 0; while (a < 10) a = a + 1; return a;"]="10"
  ["int total; int i; total = 0; for (i=1; i <= 10; i=i+1) total = total + i; return total;"]="55"
  ["int total; int i; total = 0; for (i=1; i <= 10; i=i+1) {total = total + i; total = total + i;} return total;"]="110"
  ["return f1(0);"]="2"
  ["return fact(4);"]="24"
  ["return fibo(6);"]="13"
  ["int x; int *p; x = 3; p = &x; return *p;"]="3"
  ["int *****a; return 42;"]="42"
  ["int a; int b; int c; int d; a = 1; b = 2; c = 3; d = 4; int *p; p = &c; int *q; q = p + 2; return *q;"]="1"
  ["int a; int b; int c; int d; a = 1; b = 2; c = 3; d = 4; int *p; p = &d + 2; return *p;"]="2"
  ["return sizeof(4);"]="4"
  ["int *a; return sizeof(a);"]="8"
  ["int a; a = 42; return sizeof(&a);"]="8"
  ["int a[3]; return sizeof(*a);"]="4"
  ["char c; return sizeof(c);"]="1"
  ["char c; return sizeof(c + c);"]="4"
  ["char c; int i; return sizeof(c = i);"]="1"
  ["char c; int i; return sizeof(i = c);"]="4"
  ["int a[10]; return sizeof(a);"]="40"
  ["return sizeof(sizeof(sizeof(0)));"]="4"
  ["return sizeof sizeof 4;"]="4"
  ["return sizeof(int);"]="4"
  ["return sizeof(char ***);"]="8"
  ["int *a; int b; a = &b; *a = 4; return 1;"]="1"
  ["int x; int *y; y = &x; *y = 3; return x;"]="3"
  ["int x; x = -3; int *p; p = &x; return *p+-*p*2;"]="3"
  ["int a; int b; a = b = 3; return a + b;"]="6"
  ["int a; int b; b = 4; b = b + (a = 3); return b;"]="7"
  ["int a[1]; *a = 4; return *a;"]="4"
  ["int a[2]; *a = 1; *(a + 1) = 5; return *a + *(a + 1);"]="6"
  ["int a[2]; *a = 1; *(a + 1) = 2; int *p; p = a; return *p + *(p + 1);"]="3"
  ["int a[2]; *a = 1; *(a + 1) = 2; int *p; p = a; return *(p + 1);"]="2"
  ["int a[2]; *a = 1; *(a + 1) = 2; return *(a + 1);"]="2"
  ["int a[2]; *a = 1; *(a + 1) = 2; return a[0] + a[1];"]="3"
  ["int a[2]; *a = 1; *(a + 1) = 2; return 0[a] + 1[a];"]="3"
  ["gvar = 34; return gvar;"]="34"
  ["int G; G = 34; pg = &G; return *pg;"]="34"
  ["garr[1] = 2; return garr[1];"]="2"
  ["char a; return sizeof(a);"]="1"
  ["char a; a = 3; return 3;"]="3"
  ["char x[3]; x[0] = -1; x[1] = 2; int y; y = 4; return x[0] + y;"]="3"
  ["char a; char b; char c; char d; a = 1; b = 2; c = 3; d = 4; char *p; p = &c; char *q; q = p + 2; return *q;"]="1"
  ["int a; int b; int c; int d; a = 1; b = 2; c = 3; d = 4; int *p; p = &c; int *q; q = p - 1; return *q;"]="4"
  ["char a; char b; char c; char d; a = 1; b = 2; c = 3; d = 4; char *p; p = &d + 2; return *p;"]="2"
  ["char arr[4]; return &arr[3] - &arr[0];"]="3"
  ["int arr[4]; return &arr[3] - &arr[0];"]="3"
  ["int a; int a; a = 2; int a; return a;"]="2"
  ["int a; a = 1; char b; b = 9; if (1) {int b; b = 32; a = 100; } if(1) {char b; b = 5;} return a + b;"]="109"
  ["int x[2][3]; int *y; y=x[0]; y[0]=0; return x[0][0];"]="0"
  ["int x[2][3]; int *y; y=x[0]; y[1]=1; return x[0][1];"]="1"
  ["int x[2][3]; int *y; y=x[0]; y[2]=2; return x[0][2];"]="2"
  ["int x[2][3]; int *y; y=x[1]; y[0]=3; return x[1][0];"]="3"
  ["int x[2][3]; int *y; y=x[1]; y[1]=4; return x[1][1];"]="4"
  ["int x[2][3]; int *y; y=x[1]; y[2]=5; return x[1][2];"]="5"
  ["int a[2][3]; return sizeof(*a);"]="12"
  # ["char* foo; foo = "bar"; return 0;"]="0"
  # ["return first("bcd") - first("abc");"]="1"
)

helper=$(cat <<<"int foo() { return 3; }
int add(int a, int b) {return a + b;}
int f2(int m); int f1(int n) {return f2(n+1);}
int f2(int m) {return m+1;}
int fact(int m); int fact(int n) {if (n < 2) return 1; else return n * fact(n-1);}
int fibo(int n) { if (n == 1) return 1; else if (n == 0) return 1; else return fibo(n-1) + fibo(n-2); }
int gvar;
int *pg; int *pg;
int garr[3];
char first(char *str) { return str[0]; }
")
echo -e "${helper}\n"
echo -e "int assert(int expected, int actual, char *code);\n"

i=0
for prog in "${!tests[@]}"; do
  echo "int test${i}() { ${prog} }"
  let i++
done

echo ""

echo "int main() {"
i=0
for test in "${!tests[@]}"; do
  echo "    assert(${tests[${test}]}, test${i}(), \"${test}\");"
  let i++
done
echo -e "\n    return 0;\n}"
