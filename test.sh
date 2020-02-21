#! /bin/sh

cat <<EOF | gcc -xc -c -o tmp2.o -
int ret3() { return 3; }
int ret5() { return 5; }
int add(int x, int y) { return x+y; }
int sub(int x, int y) { return x-y; }
int add6(int a, int b, int c, int d, int e, int f) {
  return a+b+c+d+e+f;
}
EOF

try() {
    expected="$1"
    input="$2"

    ./9cc "$input" > tmp.s
    gcc -o tmp tmp.s tmp2.o
    ./tmp
    actual="$?"

    if [ "$actual" = "$expected" ]; then
        echo "$input => $actual"
    else
        echo "$expected expected, but got $actual"
        exit 1
    fi
}

try 0 'return 0;'
try 42 'return 42;'
try 21 'return 5+20-4;'
try 41 "return 12 + 34 - 5; "
try 47 'return 5+6*7;'
try 15 'return 5*(9-6);'
try 4 'return (3+5)/2;'
try 10 'return -10 + 20;'
try 5 'return 20 + -15;'

try 0 'return 0 == 1;'
try 1 'return 1 == 1;'

try 0 'return 1 != 1;'
try 1 'return 1 != 4;'

try 0 'return 0 > 1;'
try 1 'return 12 > 10;'

try 1 'return 0 < 1;'
try 0 'return 11 < 10;'

try 0 'return 11 <= 10;'
try 1 'return 10 <= 10;'
try 1 'return 10 <= 11;'

try 0 'return 8 >= 15;'
try 1 'return 10 >= 10;'
try 1 'return 14 >= 4;'
try 1 'a=1; return a;'
try 3 'foo=1;return foo+2;'
try 1 'foo=1;var=3; return 1; return foo+var;'
try 4 'foo=1;var=3; return foo+var;'
try 3 'if(0) return 2; return 3;'
try 3 'if (1-1) return 2; return 3;'
try 2 'if (1) return 2; return 3;'
try 2 'if (2-1) return 2; return 3;'
try 10 'i=0; while(i<10) i=i+1; return i;'
try 55 'i=0; j=0; for (i=0; i<=10; i=i+1) j=i+j; return j;'
try 3 'for (;;) return 3; return 5;'
try 3 '{1; {2;} return 3;}'
try 55 'i=0; j=0; while(i<=10) {j=i+j; i=i+1;} return j;'
try 3 'return ret3();'
try 5 'return ret5();'
try 8 'return add(3, 5);'
try 2 'return sub(5, 3);'
try 21 'return add6(1,2,3,4,5,6);'

echo OK
