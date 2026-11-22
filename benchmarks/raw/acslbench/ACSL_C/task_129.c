/*
 * Task 129 Natural Language:
 * Write an ACSL-annotated C function that computes both the quotient (q) and remainder (r) of
 * dividing x by y. The function requires valid output pointers (q and r) that don't alias, and a
 * non-zero divisor (y). Ensures x equals q*y + r with remainder r less than y.
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