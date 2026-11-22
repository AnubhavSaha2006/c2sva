/*
 * Task 547 Natural Language:
 * Write an ACSL-annotated C function that decrements a variable `a` from a positive integer `x`
 * down to zero while incrementing a counter `y`. The function requires that `x` is a positive
 * integer less than `INT_MAX` and ensures the final value of the counter `y` equals the original
 * input `x`.
 */

#include<limits.h>
/*@
    requires x > 0 && x < INT_MAX;
    ensures \result == x;
    assigns \nothing;
*/
int test(int x) {
    int a = x;
    int y = 0;
    /*@ 
        loop invariant y + a == x;
        loop invariant a >= 0 && a <= x;
        loop assigns a, y;
        loop variant a;
    */
    while(a != 0) {
        y = y + 1;
        a = a - 1;
    }
    return y;
}
    
/*@ assigns \nothing; */
int main() {
    int num = test(3);
    //@ assert num == 3;
    return 0;
}