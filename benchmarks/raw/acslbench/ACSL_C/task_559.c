/*
 * Task 559 Natural Language:
 * Write an ACSL-annotated C function that adds two integers x and y. The function requires that
 * the sum x+y does not cause integer overflow (must be within INT_MIN and INT_MAX range) and
 * ensures the result equals the mathematical sum of x and y.
 */

#include <limits.h>
/*@
    requires x+y <= INT_MAX;
    requires x+y >= INT_MIN;
    requires x >= INT_MIN &&  y >= INT_MIN;
    ensures \result == x+y;
    assigns \nothing;
*/
int add(int x, int y) {
    return x+y;
}
