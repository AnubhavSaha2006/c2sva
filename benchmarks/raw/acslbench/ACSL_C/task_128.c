/*
 * Task 128 Natural Language:
 * Write an ACSL-annotated C function that sums three integer pointers (a, b, r) and returns their
 * total. The function requires valid pointers, separation between memory locations, and ensures
 * the sum doesn't overflow/underflow integer limits. The postcondition guarantees the result
 * equals *a + *b + *r.
 */

#include <limits.h>
/*@
    requires \valid_read(a) && \valid_read(b) && \valid_read(r);
    requires \separated(a, b, r);
    requires *a + *b + *r <= INT_MAX;
    requires *a + *b + *r >= INT_MIN;
    assigns \nothing;
    ensures \result == *a + *b + *r;
*/
int add(int *a, int *b, int *r) {
    return *a + *b + *r;
}

int main() {
    int a = 24;
    int b = 32;
    int r = 12;
    int x;

    x = add(&a, &b, &r) ;
    //@ assert x == a + b + r;
    //@ assert x == 68 ;

    x = add(&a, &a, &a) ;
    //@ assert x == a + a + a;
    //@ assert x == 72 ;
}