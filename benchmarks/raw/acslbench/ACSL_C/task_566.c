/*
 * Task 566 Natural Language:
 * Write an ACSL-annotated C function that transfers the value from variable `x` to variable `y` by
 * decrementing `x` and incrementing `y` in a loop until `x` becomes zero. The function requires a
 * positive integer input `c` and ensures the returned result equals `c`.
 */

#include <stdio.h>
/*@
    requires c > 0;
    ensures \result == c;
    assigns \nothing;
*/
int func(int c) {
    int x = c;
    int y = 0;
    /*@
        loop invariant c == x + y && x >= 0;
        loop assigns x, y;
        loop variant x;
    */
    while(x > 0) {
        x = x - 1;
        y = y + 1;
    }
    return y;
}