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

try 0 '0;'
try 42 '42;'
try 21 '5+20-4;'
try 41 " 12 + 34 - 5; "
try 47 '5+6*7;'
try 15 '5*(9-6);'
try 4 '(3+5)/2;'
try 10 '-10 + 20;'
try 5 '20 + -15;'

try 0 '0 == 1;'
try 1 '1 == 1;'

try 0 '1 != 1;'
try 1 '1 != 4;'

try 0 '0 > 1;'
try 1 '12 > 10;'

try 1 '0 < 1;'
try 0 '11 < 10;'

try 0 '11 <= 10;'
try 1 '10 <= 10;'
try 1 '10 <= 11;'

try 0 '8 >= 15;'
try 1 '10 >= 10;'
try 1 '14 >= 4;'
try 3 'a=1;a+2;'
try 3 'foo=1;foo+2;'

echo OK
