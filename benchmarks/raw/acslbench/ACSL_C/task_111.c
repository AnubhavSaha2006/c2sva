/*
 * Task 111 Natural Language:
 * Write an ACSL-annotated C function that returns the input value c (where c > 0) by counting down
 * x and accumulating in y. The function maintains the invariant that c equals x + y throughout the
 * loop.
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
    */
    while(x > 0) {
        x = x - 1;
        y = y + 1;
    }
    return y;
}