/*
 * Task 90 Natural Language:
 * Write an ACSL-annotated C function that performs integer division of x by y, storing the
 * remainder in *r. The function requires positive inputs (x ≥ y > 0) and ensures the remainder
 * (*r) is less than y, while returning the quotient that satisfies x = quotient*y + remainder.
 */

/*@
    requires x >= y && x > 0 && y > 0;
    ensures *r < y;
    ensures x == \result*y + *r;
    
*/
int fun(int x, int y , int *r) {
    *r = x;
    int d = 0;
    /*@
        loop invariant  *r == x - y*d;
        loop assigns *r, d;
    */
    while (*r >= y) {
        *r = *r - y;
        d = d + 1;
    }
    return d;
}