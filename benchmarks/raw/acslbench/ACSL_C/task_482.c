/*
 * Task 482 Natural Language:
 * Write an ACSL-annotated C function that calculates the difference between two integers x and y
 * (x - y). The function requires that neither x nor y is INT_MIN and that the result of the
 * subtraction does not cause integer overflow. The function ensures the result satisfies the
 * equation y == x - result.
 */

#include <limits.h>
/*@
    requires (x > INT_MIN) && (y > INT_MIN) && (x >= y && x - y <= INT_MAX || x < y && x - y >= INT_MIN);
    ensures y == x-\result;
    assigns \nothing;
*/
int diff (int x, int y) {
    return x-y;
}

/*@ assigns \nothing; */
void main() {
    int t = diff(10, 5);
    //@ assert t == 5;
}