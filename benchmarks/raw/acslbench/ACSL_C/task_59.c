/*
 * Task 59 Natural Language:
 * Write an ACSL-annotated C function that calculates the difference between the sum of cubes (1³
 * to n³) and the sum of numbers (1 to n) for a given non-negative integer n. The function returns
 * the result as (n²(n+1)²)/4 - (n(n+1))/2.
 */

#include <stdio.h>
#include <assert.h>

/*@
  requires n >= 0;
  assigns \nothing;
  ensures \result == (n * n * (n + 1) * (n + 1)) / 4 - (n * (n + 1)) / 2;
*/
int DifferenceSumCubesAndSumNumbers(int n) {
    int sumCubes = 0;
    int sumNumbers = 0;

    /*@
      loop invariant 1 <= i <= n+1;
      loop invariant sumCubes == ((i-1) * (i-1) * i * i) / 4;
      loop invariant sumNumbers == ((i-1) * i) / 2;
      loop assigns i, sumCubes, sumNumbers;
      loop variant n - i + 1;
    */
    for (int i = 1; i <= n; ++i) {
        sumCubes += i * i * i;
        sumNumbers += i;
    }

    return sumCubes - sumNumbers;
}