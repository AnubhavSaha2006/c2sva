/*
 * Task 91 Natural Language:
 * Write an ACSL-annotated C function that computes the absolute value of an integer parameter
 * 'val'. The function requires that 'val' is greater than INT_MIN and ensures the result is non-
 * negative, matching 'val' when positive or its negation when negative.
 */

#include <limits.h>
/*@
    requires val > INT_MIN;
    ensures positive_value: \result >= 0;
    ensures (val >= 0 ==> \result == val) && 
            (val < 0 ==> \result == -val);
*/
int abs(int val) {
    if(val < 0) return -val;
    return val;
}

void foo(int a) {
    int b = abs(-42);
    int c = abs(42);
    int d = abs(a);
    int e = abs(INT_MIN);
}