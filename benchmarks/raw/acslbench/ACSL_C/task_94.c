/*
 * Task 94 Natural Language:
 * Write an ACSL-annotated C function that calculates the difference between two integers x and y.
 * The function requires that both inputs are greater than INT_MIN to prevent overflow, and ensures
 * the result satisfies y == x - \result.
 */

#include <limits.h>
/*@
    requires (x > INT_MIN) && (y > INT_MIN);
    ensures y == x-\result;
*/


int diff (int x, int y) {
    return x-y;
}

void main() {
    int t = diff(10, 5);
    //@ assert t == 5;
}