/*
 * Task 442 Natural Language:
 * Write an ACSL-annotated C function that computes the absolute value of integer x. The function
 * requires x > -2147483648 and ensures the result is non-negative. For positive x, it returns x
 * unchanged; for negative x (except -2147483648), it returns -x. The function has no side effects.
 */



/*@
    requires x > -2147483648;
    assigns \nothing;
    ensures \result >= 0;
    behavior positive:
      assumes x >= 0;
      ensures \result == x;
    behavior negative:
      assumes x < 0;
      ensures x != -2147483648 ==> \result == -x;
    complete behaviors positive, negative;
    disjoint behaviors positive, negative;
*/
int abs(int x) {
    if (x < 0) {
        return -x;
    }
    return x;
}