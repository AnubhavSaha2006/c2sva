/*
 * Task 225 Natural Language:
 * Write an ACSL-annotated C function that performs integer division and calculates the remainder
 * of two unsigned integers x and y. The function takes x (dividend) and y (divisor, which must be
 * non-zero) as inputs, and returns the quotient via pointer q and the remainder via pointer r. The
 * function ensures the mathematical relationship x = q * y + r holds, with the remainder r being
 * less than the divisor y. The pointers q and r must point to valid, separate memory locations.
 */

/*@
    requires \valid(q) && \valid(r);
    requires \separated(q, r);
    requires y != 0;
    assigns *q, *r;
    ensures x == *q * y + *r;
    ensures *r < y;
*/
void div_rem(unsigned x, unsigned y, unsigned* q, unsigned* r) {
    *q = x / y;
    *r = x % y;
}