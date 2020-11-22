// This code is generated by test/prep.sh

// null directive
#
/* */ #

// include directive
//#include "include1.h"

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

int test0() { return 42; }
int test1() { return 1 + 1; }
int test2() { return (1 + 1) * 2; }
int test3() { return (1 + 1) / 2; }
int test4() { return (1 != 3) * 4 +  (4 - 1) == 7; }
int test5() {  5;;; return (1 + 6) / 2; }
int test6() { int a; a = 1; return a; }
int test7() { int a; a = 1 + 1; return a; }
int test8() { int a; a = 1; a = a + 1; return a; }
int test9() { int a; int b; a=1; b=1; return a + b; }
int test10() { int a; a = 1; return a + a + a; }
int test11() { int a; int b; int c; a=1; b=1; c = a + b; return a + b + c; }
int test12() { return foo(); }
int test13() { return add(1, 2); }
int test14() { if (1) return 42; }
int test15() { if (1) return 42; else return 64; }
int test16() { if (0) return 42; else return 64; }
int test17() { if (1) ; else{} return 8; }
int test18() { int num; num=10;if((num/3)*3 == num)return 3;else if((num/5)*5 == num)return 5;else return 0; }
int test19() { int a; a = 0; while (a < 10) a = a + 1; return a; }
int test20() { int total; int i; total = 0; for (i=1; i <= 10; i=i+1) total = total + i; return total; }
int test21() { int total; int i; total = 0; for (i=1; i <= 10; i=i+1) {total = total + i; total = total + i;} return total; }
int test22() { int i; i = 0; for (; i < 10;) {i = i + 1;} return i; }
int test23() { int i; i = 0; for (;;) {i = i + 1; if (i >= 5) return i;} return i; }
int test24() { return f1(0); }
int test25() { return fact(4); }
int test26() { return fibo(6); }
int test27() { int x; int *p; x = 3; p = &x; return *p; }
int test28() { int *****a; return 42; }
int test29() { int a; int b; int c; int d; a = 1; b = 2; c = 3; d = 4; int *p; p = &c; int *q; q = p + 2; return *q; }
int test30() { int a; int b; int c; int d; a = 1; b = 2; c = 3; d = 4; int *p; p = &d + 2; return *p; }
int test31() { return sizeof(4); }
int test32() { int *a; return sizeof(a); }
int test33() { int a; a = 42; return sizeof(&a); }
int test34() { int a[3]; return sizeof(*a); }
int test35() { char c; return sizeof(c); }
int test36() { char c; return sizeof(c + c); }
int test37() { char c; int i; return sizeof(c = i); }
int test38() { char c; int i; return sizeof(i = c); }
int test39() { int a[10]; return sizeof(a); }
int test40() { return sizeof(sizeof(sizeof(0))); }
int test41() { return sizeof sizeof 4; }
int test42() { return sizeof(int); }
int test43() { return sizeof(char ***); }
int test44() { int *a; int b; a = &b; *a = 4; return 1; }
int test45() { int x; int *y; y = &x; *y = 3; return x; }
int test46() { int x; x = -3; int *p; p = &x; return *p+-*p*2; }
int test47() { int a; int b; a = b = 3; return a + b; }
int test48() { int a; int b; b = 4; b = b + (a = 3); return b; }
int test49() { int a[1]; *a = 4; return *a; }
int test50() { int a[2]; *a = 1; *(a + 1) = 5; return *a + *(a + 1); }
int test51() { int a[2]; *a = 1; *(a + 1) = 2; int *p; p = a; return *p + *(p + 1); }
int test52() { int a[2]; *a = 1; *(a + 1) = 2; int *p; p = a; return *(p + 1); }
int test53() { int a[2]; *a = 1; *(a + 1) = 2; return *(a + 1); }
int test54() { int a[2]; *a = 1; *(a + 1) = 2; return a[0] + a[1]; }
int test55() { int a[2]; *a = 1; *(a + 1) = 2; return 0[a] + 1[a]; }
int test56() { gvar = 34; return gvar; }
int test57() { int G; G = 34; pg = &G; return *pg; }
int test58() { garr[1] = 2; return garr[1]; }
int test59() { char a; return sizeof(a); }
int test60() { char a; a = 3; return 3; }
int test61() { char x[3]; x[0] = -1; x[1] = 2; int y; y = 4; return x[0] + y; }
int test62() { char a; char b; char c; char d; a = 1; b = 2; c = 3; d = 4; char *p; p = &c; char *q; q = p + 2; return *q; }
int test63() { int a; int b; int c; int d; a = 1; b = 2; c = 3; d = 4; int *p; p = &c; int *q; q = p - 1; return *q; }
int test64() { char a; char b; char c; char d; a = 1; b = 2; c = 3; d = 4; char *p; p = &d + 2; return *p; }
int test65() { char arr[4]; return &arr[3] - &arr[0]; }
int test66() { int arr[4]; return &arr[3] - &arr[0]; }
int test67() { int a = 1; int b = 2; return sizeof(&a - &b); }
int test68() { int a; a = 1; char b; b = 9; if (1) {int b; b = 32; a = 100; } if(1) {char b; b = 5;} return a + b; }
int test69() { int x[2][3]; int *y; y=x[0]; y[0]=0; return x[0][0]; }
int test70() { int x[2][3]; int *y; y=x[0]; y[1]=1; return x[0][1]; }
int test71() { int x[2][3]; int *y; y=x[0]; y[2]=2; return x[0][2]; }
int test72() { int x[2][3]; int *y; y=x[1]; y[0]=3; return x[1][0]; }
int test73() { int x[2][3]; int *y; y=x[1]; y[1]=4; return x[1][1]; }
int test74() { int x[2][3]; int *y; y=x[1]; y[2]=5; return x[1][2]; }
int test75() { int a[2][3]; return sizeof(*a); }
int test76() { return 1 /*+ 1*/; }
int test77() { char* foo; foo = "bar"; return 0; }
int test78() { return first("bcd") - first("abc"); }
int test79() { return ({ 0; }); }
int test80() { return ({ 0; 1; 2; }); }
int test81() { ({ 0; return 1; 2; }); return 3; }
int test82() { return ({ 1; }) + ({ 2; }) + ({ 3; }); }
int test83() { return ({ int x; x=3; x; }); }
int test84() { return ({ int a; a=3; a = a + 3; }); }
int test85() { return ({ int a; a = 0; while (a < 10) a = a + 1; a; }); }
int test86() { return ({ int total; int i; total = 0; for (i=1; i <= 10; i=i+1) total = total + i; total; }); }
int test87() { return add(1, ({ int a[3] = {0, 1, 2}; a[1]; })); }
int test88() { return add(0, ({ int i = 0; for (;i < 10;) {i = i + 1; } i; })); }
int test89() { int x = 3; return x; }
int test90() { int x = foo(); return x; }
int test91() { char x = 3; return x; }
int test92() { int x = {3}; return x; }
int test93() { int a[3] = {0, 1, 2}; return a[0]; }
int test94() { int a[3] = {0, 1, 2}; return a[1]; }
int test95() { int a[3] = {0, 1, 2}; return a[2]; }
int test96() { int a[3] = {3}; return a[0]; }
int test97() { int a[3] = {3}; return a[1]; }
int test98() { int a[3] = {3}; return a[2]; }
int test99() { int a[] = {0, 1, 2}; return a[0]; }
int test100() { int a[] = {0, 1, 2}; return a[1]; }
int test101() { int a[] = {0, 1, 2}; return a[2]; }
int test102() { int a[2][3] = {{0, 1, 2}, {3, 4, 5}}; return a[1][1]; }
int test103() { int a[][3] = {{0, 1, 2}, {3, 4, 5}}; return a[1][0]; }
int test104() { int a[][3] = {{1, 2}, {3}}; return a[0][0]; }
int test105() { int a[][3] = {{1, 2}, {3}}; return a[0][1]; }
int test106() { int a[][3] = {{1, 2}, {3}}; return a[0][2]; }
int test107() { int a[][3] = {{1, 2}, {3}}; return a[1][0]; }
int test108() { int a[][3] = {{1, 2}, {3}}; return a[1][1]; }
int test109() { int a[][3] = {{1, 2}, {3}}; return a[1][2]; }
int test110() { int a[][2][3] = {{{}, {1, 2, 3}}, {{}, {}}}; return a[0][1][2]; }
int test111() { char a[4] = "ABC"; return a[0]; }
int test112() { char a[4] = "ABC"; return a[1]; }
int test113() { char a[4] = "ABC"; return a[2]; }
int test114() { char a[4] = "ABC"; return a[3]; }
int test115() { char str[] = "hello"; return sizeof(str); }
int test116() { char c = 3; return add(c, c + 1); }
int test117() { return add(0, first("ABC")); }
int test118() { int x = 2; x++; return x; }
int test119() { int x = 2; x--; return x; }
int test120() { int x = 2; return x++; }
int test121() { int x = 2; return x--; }
int test122() { int x = 2; return ++x; }
int test123() { int x = 2; return --x; }
int test124() { int x = 2; return x++ + ++x; }
int test125() { int x = 2; int *p = &x; ++*p; return x; }
int test126() { struct {int a; char b;} X; return sizeof(X); }
int test127() { struct {int a; char b;} X[3]; return sizeof(X); }
int test128() { struct {int a; char b;} X; X.a = 1; X.b = 2; return X.a; }
int test129() { struct {int a; char b;} X; X.a = 1; X.b = 2; return X.b; }
int test130() { struct X {int a; char b;}; struct X x; x.a = 1; x.b = 2; return x.a; }
int test131() { struct X {int a; char b;}; struct X x; x.a = 1; x.b = 2; return x.b; }
int test132() { struct Vector {int x; int y;}; struct Vector a; struct Vector *pa = &a; pa->x = 1; pa->y = 2; return pa->x; }
int test133() { struct Vector {int x; int y;}; struct Vector a; struct Vector *pa = &a; pa->x = 1; pa->y = 2; return pa->y; }
int test134() { struct Vector {int x; int y;}; struct Vector a; struct Vector *pa = &a; pa->x = 1; return a.x; }
int test135() { int a = 0; a += 1; return a; }
int test136() { int a = 2; a -= 1; return a; }
int test137() { int a = 0; return a += 1; }
int test138() { int a = 2; return a -= 1; }
int test139() { int arr[] = {0, 1, 2}; int *p = arr; p += 1; return *p; }
int test140() { int arr[] = {0, 1, 2}; int *p = arr; p += 2; return *p; }
int test141() { int a = 14; a *= 3; return a; }
int test142() { int a = 42; a /= 3; return a; }

int main() {
    assert(42, test0(), "{ return 42; }");
    assert(2, test1(), "{ return 1 + 1; }");
    assert(4, test2(), "{ return (1 + 1) * 2; }");
    assert(1, test3(), "{ return (1 + 1) / 2; }");
    assert(1, test4(), "{ return (1 != 3) * 4 +  (4 - 1) == 7; }");
    assert(3, test5(), "{  5;;; return (1 + 6) / 2; }");
    assert(1, test6(), "{ int a; a = 1; return a; }");
    assert(2, test7(), "{ int a; a = 1 + 1; return a; }");
    assert(2, test8(), "{ int a; a = 1; a = a + 1; return a; }");
    assert(2, test9(), "{ int a; int b; a=1; b=1; return a + b; }");
    assert(3, test10(), "{ int a; a = 1; return a + a + a; }");
    assert(4, test11(), "{ int a; int b; int c; a=1; b=1; c = a + b; return a + b + c; }");
    assert(3, test12(), "{ return foo(); }");
    assert(3, test13(), "{ return add(1, 2); }");
    assert(42, test14(), "{ if (1) return 42; }");
    assert(42, test15(), "{ if (1) return 42; else return 64; }");
    assert(64, test16(), "{ if (0) return 42; else return 64; }");
    assert(8, test17(), "{ if (1) ; else{} return 8; }");
    assert(5, test18(), "{ int num; num=10;if((num/3)*3 == num)return 3;else if((num/5)*5 == num)return 5;else return 0; }");
    assert(10, test19(), "{ int a; a = 0; while (a < 10) a = a + 1; return a; }");
    assert(55, test20(), "{ int total; int i; total = 0; for (i=1; i <= 10; i=i+1) total = total + i; return total; }");
    assert(110, test21(), "{ int total; int i; total = 0; for (i=1; i <= 10; i=i+1) {total = total + i; total = total + i;} return total; }");
    assert(10, test22(), "{ int i; i = 0; for (; i < 10;) {i = i + 1;} return i; }");
    assert(5, test23(), "{ int i; i = 0; for (;;) {i = i + 1; if (i >= 5) return i;} return i; }");
    assert(2, test24(), "{ return f1(0); }");
    assert(24, test25(), "{ return fact(4); }");
    assert(13, test26(), "{ return fibo(6); }");
    assert(3, test27(), "{ int x; int *p; x = 3; p = &x; return *p; }");
    assert(42, test28(), "{ int *****a; return 42; }");
    assert(1, test29(), "{ int a; int b; int c; int d; a = 1; b = 2; c = 3; d = 4; int *p; p = &c; int *q; q = p + 2; return *q; }");
    assert(2, test30(), "{ int a; int b; int c; int d; a = 1; b = 2; c = 3; d = 4; int *p; p = &d + 2; return *p; }");
    assert(4, test31(), "{ return sizeof(4); }");
    assert(8, test32(), "{ int *a; return sizeof(a); }");
    assert(8, test33(), "{ int a; a = 42; return sizeof(&a); }");
    assert(4, test34(), "{ int a[3]; return sizeof(*a); }");
    assert(1, test35(), "{ char c; return sizeof(c); }");
    assert(4, test36(), "{ char c; return sizeof(c + c); }");
    assert(1, test37(), "{ char c; int i; return sizeof(c = i); }");
    assert(4, test38(), "{ char c; int i; return sizeof(i = c); }");
    assert(40, test39(), "{ int a[10]; return sizeof(a); }");
    assert(4, test40(), "{ return sizeof(sizeof(sizeof(0))); }");
    assert(4, test41(), "{ return sizeof sizeof 4; }");
    assert(4, test42(), "{ return sizeof(int); }");
    assert(8, test43(), "{ return sizeof(char ***); }");
    assert(1, test44(), "{ int *a; int b; a = &b; *a = 4; return 1; }");
    assert(3, test45(), "{ int x; int *y; y = &x; *y = 3; return x; }");
    assert(3, test46(), "{ int x; x = -3; int *p; p = &x; return *p+-*p*2; }");
    assert(6, test47(), "{ int a; int b; a = b = 3; return a + b; }");
    assert(7, test48(), "{ int a; int b; b = 4; b = b + (a = 3); return b; }");
    assert(4, test49(), "{ int a[1]; *a = 4; return *a; }");
    assert(6, test50(), "{ int a[2]; *a = 1; *(a + 1) = 5; return *a + *(a + 1); }");
    assert(3, test51(), "{ int a[2]; *a = 1; *(a + 1) = 2; int *p; p = a; return *p + *(p + 1); }");
    assert(2, test52(), "{ int a[2]; *a = 1; *(a + 1) = 2; int *p; p = a; return *(p + 1); }");
    assert(2, test53(), "{ int a[2]; *a = 1; *(a + 1) = 2; return *(a + 1); }");
    assert(3, test54(), "{ int a[2]; *a = 1; *(a + 1) = 2; return a[0] + a[1]; }");
    assert(3, test55(), "{ int a[2]; *a = 1; *(a + 1) = 2; return 0[a] + 1[a]; }");
    assert(34, test56(), "{ gvar = 34; return gvar; }");
    assert(34, test57(), "{ int G; G = 34; pg = &G; return *pg; }");
    assert(2, test58(), "{ garr[1] = 2; return garr[1]; }");
    assert(1, test59(), "{ char a; return sizeof(a); }");
    assert(3, test60(), "{ char a; a = 3; return 3; }");
    assert(3, test61(), "{ char x[3]; x[0] = -1; x[1] = 2; int y; y = 4; return x[0] + y; }");
    assert(1, test62(), "{ char a; char b; char c; char d; a = 1; b = 2; c = 3; d = 4; char *p; p = &c; char *q; q = p + 2; return *q; }");
    assert(4, test63(), "{ int a; int b; int c; int d; a = 1; b = 2; c = 3; d = 4; int *p; p = &c; int *q; q = p - 1; return *q; }");
    assert(2, test64(), "{ char a; char b; char c; char d; a = 1; b = 2; c = 3; d = 4; char *p; p = &d + 2; return *p; }");
    assert(3, test65(), "{ char arr[4]; return &arr[3] - &arr[0]; }");
    assert(3, test66(), "{ int arr[4]; return &arr[3] - &arr[0]; }");
    assert(4, test67(), "{ int a = 1; int b = 2; return sizeof(&a - &b); }");
    assert(109, test68(), "{ int a; a = 1; char b; b = 9; if (1) {int b; b = 32; a = 100; } if(1) {char b; b = 5;} return a + b; }");
    assert(0, test69(), "{ int x[2][3]; int *y; y=x[0]; y[0]=0; return x[0][0]; }");
    assert(1, test70(), "{ int x[2][3]; int *y; y=x[0]; y[1]=1; return x[0][1]; }");
    assert(2, test71(), "{ int x[2][3]; int *y; y=x[0]; y[2]=2; return x[0][2]; }");
    assert(3, test72(), "{ int x[2][3]; int *y; y=x[1]; y[0]=3; return x[1][0]; }");
    assert(4, test73(), "{ int x[2][3]; int *y; y=x[1]; y[1]=4; return x[1][1]; }");
    assert(5, test74(), "{ int x[2][3]; int *y; y=x[1]; y[2]=5; return x[1][2]; }");
    assert(12, test75(), "{ int a[2][3]; return sizeof(*a); }");
    assert(1, test76(), "{ return 1 /*+ 1*/; }");
    assert(0, test77(), "{ char* foo; foo = \"bar\"; return 0; }");
    assert(1, test78(), "{ return first(\"bcd\") - first(\"abc\"); }");
    assert(0, test79(), "{ return ({ 0; }); }");
    assert(2, test80(), "{ return ({ 0; 1; 2; }); }");
    assert(1, test81(), "{ ({ 0; return 1; 2; }); return 3; }");
    assert(6, test82(), "{ return ({ 1; }) + ({ 2; }) + ({ 3; }); }");
    assert(3, test83(), "{ return ({ int x; x=3; x; }); }");
    assert(6, test84(), "{ return ({ int a; a=3; a = a + 3; }); }");
    assert(10, test85(), "{ return ({ int a; a = 0; while (a < 10) a = a + 1; a; }); }");
    assert(55, test86(), "{ return ({ int total; int i; total = 0; for (i=1; i <= 10; i=i+1) total = total + i; total; }); }");
    assert(2, test87(), "{ return add(1, ({ int a[3] = {0, 1, 2}; a[1]; })); }");
    assert(10, test88(), "{ return add(0, ({ int i = 0; for (;i < 10;) {i = i + 1; } i; })); }");
    assert(3, test89(), "{ int x = 3; return x; }");
    assert(3, test90(), "{ int x = foo(); return x; }");
    assert(3, test91(), "{ char x = 3; return x; }");
    assert(3, test92(), "{ int x = {3}; return x; }");
    assert(0, test93(), "{ int a[3] = {0, 1, 2}; return a[0]; }");
    assert(1, test94(), "{ int a[3] = {0, 1, 2}; return a[1]; }");
    assert(2, test95(), "{ int a[3] = {0, 1, 2}; return a[2]; }");
    assert(3, test96(), "{ int a[3] = {3}; return a[0]; }");
    assert(0, test97(), "{ int a[3] = {3}; return a[1]; }");
    assert(0, test98(), "{ int a[3] = {3}; return a[2]; }");
    assert(0, test99(), "{ int a[] = {0, 1, 2}; return a[0]; }");
    assert(1, test100(), "{ int a[] = {0, 1, 2}; return a[1]; }");
    assert(2, test101(), "{ int a[] = {0, 1, 2}; return a[2]; }");
    assert(4, test102(), "{ int a[2][3] = {{0, 1, 2}, {3, 4, 5}}; return a[1][1]; }");
    assert(3, test103(), "{ int a[][3] = {{0, 1, 2}, {3, 4, 5}}; return a[1][0]; }");
    assert(1, test104(), "{ int a[][3] = {{1, 2}, {3}}; return a[0][0]; }");
    assert(2, test105(), "{ int a[][3] = {{1, 2}, {3}}; return a[0][1]; }");
    assert(0, test106(), "{ int a[][3] = {{1, 2}, {3}}; return a[0][2]; }");
    assert(3, test107(), "{ int a[][3] = {{1, 2}, {3}}; return a[1][0]; }");
    assert(0, test108(), "{ int a[][3] = {{1, 2}, {3}}; return a[1][1]; }");
    assert(0, test109(), "{ int a[][3] = {{1, 2}, {3}}; return a[1][2]; }");
    assert(3, test110(), "{ int a[][2][3] = {{{}, {1, 2, 3}}, {{}, {}}}; return a[0][1][2]; }");
    assert(65, test111(), "{ char a[4] = \"ABC\"; return a[0]; }");
    assert(66, test112(), "{ char a[4] = \"ABC\"; return a[1]; }");
    assert(67, test113(), "{ char a[4] = \"ABC\"; return a[2]; }");
    assert(0, test114(), "{ char a[4] = \"ABC\"; return a[3]; }");
    assert(6, test115(), "{ char str[] = \"hello\"; return sizeof(str); }");
    assert(7, test116(), "{ char c = 3; return add(c, c + 1); }");
    assert(65, test117(), "{ return add(0, first(\"ABC\")); }");
    assert(3, test118(), "{ int x = 2; x++; return x; }");
    assert(1, test119(), "{ int x = 2; x--; return x; }");
    assert(2, test120(), "{ int x = 2; return x++; }");
    assert(2, test121(), "{ int x = 2; return x--; }");
    assert(3, test122(), "{ int x = 2; return ++x; }");
    assert(1, test123(), "{ int x = 2; return --x; }");
    assert(6, test124(), "{ int x = 2; return x++ + ++x; }");
    assert(3, test125(), "{ int x = 2; int *p = &x; ++*p; return x; }");
    assert(5, test126(), "{ struct {int a; char b;} X; return sizeof(X); }");
    assert(15, test127(), "{ struct {int a; char b;} X[3]; return sizeof(X); }");
    assert(1, test128(), "{ struct {int a; char b;} X; X.a = 1; X.b = 2; return X.a; }");
    assert(2, test129(), "{ struct {int a; char b;} X; X.a = 1; X.b = 2; return X.b; }");
    assert(1, test130(), "{ struct X {int a; char b;}; struct X x; x.a = 1; x.b = 2; return x.a; }");
    assert(2, test131(), "{ struct X {int a; char b;}; struct X x; x.a = 1; x.b = 2; return x.b; }");
    assert(1, test132(), "{ struct Vector {int x; int y;}; struct Vector a; struct Vector *pa = &a; pa->x = 1; pa->y = 2; return pa->x; }");
    assert(2, test133(), "{ struct Vector {int x; int y;}; struct Vector a; struct Vector *pa = &a; pa->x = 1; pa->y = 2; return pa->y; }");
    assert(1, test134(), "{ struct Vector {int x; int y;}; struct Vector a; struct Vector *pa = &a; pa->x = 1; return a.x; }");
    assert(1, test135(), "{ int a = 0; a += 1; return a; }");
    assert(1, test136(), "{ int a = 2; a -= 1; return a; }");
    assert(1, test137(), "{ int a = 0; return a += 1; }");
    assert(1, test138(), "{ int a = 2; return a -= 1; }");
    assert(1, test139(), "{ int arr[] = {0, 1, 2}; int *p = arr; p += 1; return *p; }");
    assert(2, test140(), "{ int arr[] = {0, 1, 2}; int *p = arr; p += 2; return *p; }");
    assert(42, test141(), "{ int a = 14; a *= 3; return a; }");
    assert(14, test142(), "{ int a = 42; a /= 3; return a; }");

    return 0;
}
