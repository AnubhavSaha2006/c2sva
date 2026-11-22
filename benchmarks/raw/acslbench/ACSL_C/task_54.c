/*
 * Task 54 Natural Language:
 * Write an ACSL-annotated C function that calculates the sum of all common divisors of two
 * positive integers a and b. The function requires both inputs to be positive (a > 0 && b > 0) and
 * returns the sum of all integers that divide both a and b.
 */

#include <stdio.h>
#include <assert.h>

/*@
  requires a > 0 && b > 0;
  assigns \nothing;
  ensures \result >= 0;
  ensures \result == \sum(1, (a < b ? a : b), \lambda integer i; 
          (a % i == 0 && b % i == 0) ? i : 0);
*/
int SumOfCommonDivisors(int a, int b) {
    int sum = 0;
    int i = 1;
    int min = a < b ? a : b;

    /*@
      loop invariant 1 <= i <= min + 1;
      loop invariant sum >= 0;
      loop invariant sum == \sum(1, i-1, \lambda integer k;
                            (a % k == 0 && b % k == 0) ? k : 0);
      loop assigns i, sum;
      loop variant min - i + 1;
    */
    while (i <= min) {
        if (a % i == 0 && b % i == 0) {
            sum += i;
        }
        i++;
    }

    return sum;
}