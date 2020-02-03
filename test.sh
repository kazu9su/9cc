#! /bin/sh

########################################
#
# Author:
# Last Change: 23-Mar-2013.
#
# Ref:
#
########################################

try() {
    expected="$1"
    input="$2"

    ./9cc "$input" > tmp.s
    gcc -o tmp tmp.s
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

echo OK
