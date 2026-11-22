/*
 * Task 114 Natural Language:
 * Write an ACSL-annotated C function that multiplies two non-negative integers (a and b) using
 * repeated addition. The function requires both inputs to be non-negative and ensures the result
 * equals a*b. The implementation uses a while loop that maintains the product invariant through
 * additions.
 */

#include <stdio.h>
/*@
    requires a >= 0 && b >= 0;
    ensures \result == \old(a)*\old(b);
    assigns \nothing;
*/
int mul(int a, int b) {
    int x = a, y = b, prod = 0;
    /*@ 
        loop invariant prod == (a-x)*y;
        loop assigns prod, x;
    */
    while(x >= 0) {
        prod = prod + y;
        x--;
    }
    return prod;
}

int main() {
    int pdt = mul(2, 5);
    //@ assert pdt == 10;
}