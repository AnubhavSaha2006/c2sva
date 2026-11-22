/*
 * Task 548 Natural Language:
 * Write an ACSL-annotated C function that calculates the sum of squares of integers from 1 to n.
 * The function requires that n is an integer between 1 and 10 (0 < n ≤ 10) and ensures the result
 * equals the mathematical formula n*(n+1)*(2*n+1)/6.
 */

/*@
    requires 0 < n <= 10;
    assigns \nothing;
    ensures \result == n * (n + 1) * (2 * n + 1) / 6;
*/
int sq(int n)
{
    int s;
    int i;
    s = 0;
    i = 1;
    /*@
        loop invariant 1 <= i <= n + 1;
        loop invariant s == (i - 1) * i * (2 * i - 1) / 6;
        loop assigns s, i;
        loop variant n - i;
    */
    while (i <= n)
    {
        s = s + i * i;
        i += 1;
    }
    return s;
}