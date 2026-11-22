/*
 * Task 112 Natural Language:
 * Write an ACSL-annotated C function `fun` that calculates the sum of powers of 2 from 2⁰ to 2ⁿ
 * (where n is the input parameter). The function ensures the result equals (2^(n+1)) - 1 and
 * requires non-negative input n.
 */

#include <stdio.h>
#include <math.h>
/*@
    requires n >= 0;
    ensures \result == (\pow(2, n+1)) - 1;
    assigns \nothing;
*/
int fun(int n) {
    double y = 0;
    double i = 0;
    /*@
        loop invariant (y == (\pow(2, i)) - 1) && (i <= n);
        loop assigns i, y;
    */
    while(i <= n) {
        y = y + pow(2.0, i);
        i = i + 1;
    }
    return y;
}

int main() {
    int res = fun(4);
    //@assert res == 7;
}