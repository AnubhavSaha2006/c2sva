/*
 * Task 127 Natural Language:
 * Write an ACSL-annotated C function that safely adds two integers referenced by pointers p and q.
 * The function requires valid separated pointers and checks for integer overflow/underflow.
 * Returns the sum of *p and *q.
 */

#include <limits.h>
/*@
    requires \valid_read(p) && \valid_read(q);
    requires \separated(p, q);
    requires *p + *q <= INT_MAX;
    requires *p + *q >= INT_MIN;
    assigns \nothing;
    ensures \result == *p + *q;
*/
int add(int *p, int *q) {
    return *p + *q;
}

int main() {
    int a = 24;
    int b = 32;
    int x;

    x = add(&a, &b) ;
    //@ assert x == a + b ;
    //@ assert x == 56 ;

    x = add(&a, &a) ;
    //@ assert x == a + a ;
    //@ assert x == 48 ;
}