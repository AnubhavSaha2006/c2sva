/*
 * Task 116 Natural Language:
 * Write an ACSL-annotated C function that computes the sum of a mathematical series. The function
 * takes an integer parameter `n` and returns the result of the formula `0.25*n*(n+2)`. The
 * function requires `n` to be non-negative.
 */

/*@
    requires n>=0;
    ensures \result == 0.25*n*(n+2);
    assigns \nothing;
*/
int func(int n) {
    int sum = 0;
    int i = 0;
    /*@
        loop invariant (sum == i*(i-1));
        loop assigns sum, i;
    */
    while(i <= n/2) {
        sum = sum + 2*(i);
        i++;
    }
    return sum;
}