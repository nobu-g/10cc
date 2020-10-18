// This code is generated by test/prep.sh

int assert(int expected, int actual, char *code);

int foo() { return 3; }
int add(int a, int b) {return a + b;}
int f2(int m); int f1(int n) {return f2(n+1);}
int f2(int m) {return m+1;}
int fact(int m); int fact(int n) {if (n < 2) return 1; else return n * fact(n-1);}
int fibo(int n) { if (n == 1) return 1; else if (n == 0) return 1; else return fibo(n-1) + fibo(n-2); }
int gvar;
int *pg; int *pg;
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
int test22() { return f1(0); }
int test23() { return fact(4); }
int test24() { return fibo(6); }
int test25() { int x; int *p; x = 3; p = &x; return *p; }
int test26() { int *****a; return 42; }
int test27() { int a; int b; int c; int d; a = 1; b = 2; c = 3; d = 4; int *p; p = &c; int *q; q = p + 2; return *q; }
int test28() { int a; int b; int c; int d; a = 1; b = 2; c = 3; d = 4; int *p; p = &d + 2; return *p; }
int test29() { return sizeof(4); }
int test30() { int *a; return sizeof(a); }
int test31() { int a; a = 42; return sizeof(&a); }
int test32() { int a[3]; return sizeof(*a); }
int test33() { char c; return sizeof(c); }
int test34() { char c; return sizeof(c + c); }
int test35() { char c; int i; return sizeof(c = i); }
int test36() { char c; int i; return sizeof(i = c); }
int test37() { int a[10]; return sizeof(a); }
int test38() { return sizeof(sizeof(sizeof(0))); }
int test39() { return sizeof sizeof 4; }
int test40() { return sizeof(int); }
int test41() { return sizeof(char ***); }
int test42() { int *a; int b; a = &b; *a = 4; return 1; }
int test43() { int x; int *y; y = &x; *y = 3; return x; }
int test44() { int x; x = -3; int *p; p = &x; return *p+-*p*2; }
int test45() { int a; int b; a = b = 3; return a + b; }
int test46() { int a; int b; b = 4; b = b + (a = 3); return b; }
int test47() { int a[1]; *a = 4; return *a; }
int test48() { int a[2]; *a = 1; *(a + 1) = 5; return *a + *(a + 1); }
int test49() { int a[2]; *a = 1; *(a + 1) = 2; int *p; p = a; return *p + *(p + 1); }
int test50() { int a[2]; *a = 1; *(a + 1) = 2; int *p; p = a; return *(p + 1); }
int test51() { int a[2]; *a = 1; *(a + 1) = 2; return *(a + 1); }
int test52() { int a[2]; *a = 1; *(a + 1) = 2; return a[0] + a[1]; }
int test53() { int a[2]; *a = 1; *(a + 1) = 2; return 0[a] + 1[a]; }
int test54() { gvar = 34; return gvar; }
int test55() { int G; G = 34; pg = &G; return *pg; }
int test56() { garr[1] = 2; return garr[1]; }
int test57() { char a; return sizeof(a); }
int test58() { char a; a = 3; return 3; }
int test59() { char x[3]; x[0] = -1; x[1] = 2; int y; y = 4; return x[0] + y; }
int test60() { char a; char b; char c; char d; a = 1; b = 2; c = 3; d = 4; char *p; p = &c; char *q; q = p + 2; return *q; }
int test61() { int a; int b; int c; int d; a = 1; b = 2; c = 3; d = 4; int *p; p = &c; int *q; q = p - 1; return *q; }
int test62() { char a; char b; char c; char d; a = 1; b = 2; c = 3; d = 4; char *p; p = &d + 2; return *p; }
int test63() { char arr[4]; return &arr[3] - &arr[0]; }
int test64() { int arr[4]; return &arr[3] - &arr[0]; }
int test65() { int a; int a; a = 2; int a; return a; }
int test66() { int a; a = 1; char b; b = 9; if (1) {int b; b = 32; a = 100; } if(1) {char b; b = 5;} return a + b; }
int test67() { int x[2][3]; int *y; y=x[0]; y[0]=0; return x[0][0]; }
int test68() { int x[2][3]; int *y; y=x[0]; y[1]=1; return x[0][1]; }
int test69() { int x[2][3]; int *y; y=x[0]; y[2]=2; return x[0][2]; }
int test70() { int x[2][3]; int *y; y=x[1]; y[0]=3; return x[1][0]; }
int test71() { int x[2][3]; int *y; y=x[1]; y[1]=4; return x[1][1]; }
int test72() { int x[2][3]; int *y; y=x[1]; y[2]=5; return x[1][2]; }
int test73() { int a[2][3]; return sizeof(*a); }
int test74() { return 1 /*+ 1*/; }

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
    assert(2, test22(), "{ return f1(0); }");
    assert(24, test23(), "{ return fact(4); }");
    assert(13, test24(), "{ return fibo(6); }");
    assert(3, test25(), "{ int x; int *p; x = 3; p = &x; return *p; }");
    assert(42, test26(), "{ int *****a; return 42; }");
    assert(1, test27(), "{ int a; int b; int c; int d; a = 1; b = 2; c = 3; d = 4; int *p; p = &c; int *q; q = p + 2; return *q; }");
    assert(2, test28(), "{ int a; int b; int c; int d; a = 1; b = 2; c = 3; d = 4; int *p; p = &d + 2; return *p; }");
    assert(4, test29(), "{ return sizeof(4); }");
    assert(8, test30(), "{ int *a; return sizeof(a); }");
    assert(8, test31(), "{ int a; a = 42; return sizeof(&a); }");
    assert(4, test32(), "{ int a[3]; return sizeof(*a); }");
    assert(1, test33(), "{ char c; return sizeof(c); }");
    assert(4, test34(), "{ char c; return sizeof(c + c); }");
    assert(1, test35(), "{ char c; int i; return sizeof(c = i); }");
    assert(4, test36(), "{ char c; int i; return sizeof(i = c); }");
    assert(40, test37(), "{ int a[10]; return sizeof(a); }");
    assert(4, test38(), "{ return sizeof(sizeof(sizeof(0))); }");
    assert(4, test39(), "{ return sizeof sizeof 4; }");
    assert(4, test40(), "{ return sizeof(int); }");
    assert(8, test41(), "{ return sizeof(char ***); }");
    assert(1, test42(), "{ int *a; int b; a = &b; *a = 4; return 1; }");
    assert(3, test43(), "{ int x; int *y; y = &x; *y = 3; return x; }");
    assert(3, test44(), "{ int x; x = -3; int *p; p = &x; return *p+-*p*2; }");
    assert(6, test45(), "{ int a; int b; a = b = 3; return a + b; }");
    assert(7, test46(), "{ int a; int b; b = 4; b = b + (a = 3); return b; }");
    assert(4, test47(), "{ int a[1]; *a = 4; return *a; }");
    assert(6, test48(), "{ int a[2]; *a = 1; *(a + 1) = 5; return *a + *(a + 1); }");
    assert(3, test49(), "{ int a[2]; *a = 1; *(a + 1) = 2; int *p; p = a; return *p + *(p + 1); }");
    assert(2, test50(), "{ int a[2]; *a = 1; *(a + 1) = 2; int *p; p = a; return *(p + 1); }");
    assert(2, test51(), "{ int a[2]; *a = 1; *(a + 1) = 2; return *(a + 1); }");
    assert(3, test52(), "{ int a[2]; *a = 1; *(a + 1) = 2; return a[0] + a[1]; }");
    assert(3, test53(), "{ int a[2]; *a = 1; *(a + 1) = 2; return 0[a] + 1[a]; }");
    assert(34, test54(), "{ gvar = 34; return gvar; }");
    assert(34, test55(), "{ int G; G = 34; pg = &G; return *pg; }");
    assert(2, test56(), "{ garr[1] = 2; return garr[1]; }");
    assert(1, test57(), "{ char a; return sizeof(a); }");
    assert(3, test58(), "{ char a; a = 3; return 3; }");
    assert(3, test59(), "{ char x[3]; x[0] = -1; x[1] = 2; int y; y = 4; return x[0] + y; }");
    assert(1, test60(), "{ char a; char b; char c; char d; a = 1; b = 2; c = 3; d = 4; char *p; p = &c; char *q; q = p + 2; return *q; }");
    assert(4, test61(), "{ int a; int b; int c; int d; a = 1; b = 2; c = 3; d = 4; int *p; p = &c; int *q; q = p - 1; return *q; }");
    assert(2, test62(), "{ char a; char b; char c; char d; a = 1; b = 2; c = 3; d = 4; char *p; p = &d + 2; return *p; }");
    assert(3, test63(), "{ char arr[4]; return &arr[3] - &arr[0]; }");
    assert(3, test64(), "{ int arr[4]; return &arr[3] - &arr[0]; }");
    assert(2, test65(), "{ int a; int a; a = 2; int a; return a; }");
    assert(109, test66(), "{ int a; a = 1; char b; b = 9; if (1) {int b; b = 32; a = 100; } if(1) {char b; b = 5;} return a + b; }");
    assert(0, test67(), "{ int x[2][3]; int *y; y=x[0]; y[0]=0; return x[0][0]; }");
    assert(1, test68(), "{ int x[2][3]; int *y; y=x[0]; y[1]=1; return x[0][1]; }");
    assert(2, test69(), "{ int x[2][3]; int *y; y=x[0]; y[2]=2; return x[0][2]; }");
    assert(3, test70(), "{ int x[2][3]; int *y; y=x[1]; y[0]=3; return x[1][0]; }");
    assert(4, test71(), "{ int x[2][3]; int *y; y=x[1]; y[1]=4; return x[1][1]; }");
    assert(5, test72(), "{ int x[2][3]; int *y; y=x[1]; y[2]=5; return x[1][2]; }");
    assert(12, test73(), "{ int a[2][3]; return sizeof(*a); }");
    assert(1, test74(), "{ return 1 /*+ 1*/; }");

    return 0;
}