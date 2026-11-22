/*
 * Task 93 Natural Language:
 * Write an ACSL-annotated C function `fun` that calculates a value based on integer input `n`. The
 * function requires `n > 0` and ensures the result equals `(n-4)/3.0`. The implementation uses a
 * loop that maintains the invariant `i == 4 + 3*x`.
 */

#include <stdio.h>
/*@
    requires n > 0;
    ensures \result == (n-4)/3.0;
    assigns \nothing;
*/
int fun(int n) {
    int i = 7;
    int x = 1;
    /*@
        loop invariant i == 4 + 3*x;
        loop assigns x, i;
    */
    while(i <= n) {
        x += 1;
        i += 3;
    }
    return x;
}

int main() {
    int a = fun(20);
    printf("%d\n", a);
}