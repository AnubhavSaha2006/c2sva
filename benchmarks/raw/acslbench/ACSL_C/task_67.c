/*
 * Task 67 Natural Language:
 * Write an ACSL-annotated C function that calculates the sum and average of the first n natural
 * numbers. The function takes input n (must be positive) and stores results in *sum and *avg
 * parameters. The postconditions guarantee correct mathematical results for both values.
 */

#include <stdio.h>
#include <assert.h>

/*@
  requires n > 0;
  assigns \nothing;
  ensures *sum == n * (n + 1) / 2;
  ensures *avg == (n * (n + 1) / 2) / n;
*/
void SumAndAverage(int n, int* sum, double* avg) {
    *sum = 0;
    
    /*@
      loop invariant 1 <= i <= n+1;
      loop invariant *sum == (i-1)*i/2;
      loop assigns i, *sum;
      loop variant n - i + 1;
    */
    for (int i = 1; i <= n; ++i) {
        *sum += i;
    }

    *avg = (double)(*sum) / n;
}