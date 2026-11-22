/*
 * Task 474 Natural Language:
 * Write an ACSL-annotated C function that performs integer division of x by y, returning the
 * quotient and storing the remainder in *r. The function requires x >= y > 0 and a valid pointer
 * r. It ensures that the remainder *r is less than y and that the equation x = quotient * y + *r
 * holds.
 */

/*@
    requires x >= y > 0;
    requires \valid (r);
    ensures *r < y;
    ensures x == \result*y + *r;
    assigns *r;
*/
int fun(int x, int y , int *r) {
    *r = x;
    int d = 0;
    /*@
        loop invariant  *r == x - y*d;
        loop invariant d >= 0;
        loop assigns *r, d;
        loop variant *r;
    */
    while (*r >= y) {
        *r = *r - y;
        d = d + 1;
    }
    return d;
}