/*
 * Task 92 Natural Language:
 * Write an ACSL-annotated C function `add(int x, int y)` that safely computes the sum of two
 * integers. The function requires that the sum doesn't overflow (x+y ≤ INT_MAX) and both inputs
 * are within integer bounds (≥ INT_MIN). Ensures the result equals x+y.
 */

#include <limits.h>
/*@
    requires x+y <= INT_MAX;
    requires x >= INT_MIN &&  y >= INT_MIN;
    ensures \result == x+y;
*/
int add(int x, int y) {
    return x+y;
}

void foo() {
    int a = add(1, 43);
    int b = add(INT_MAX, INT_MAX);
}