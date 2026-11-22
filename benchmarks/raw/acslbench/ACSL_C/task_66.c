/*
 * Task 66 Natural Language:
 * Write an ACSL-annotated C function that returns the difference between the first even and first
 * odd elements in array `a` of size `n`. Requires the array to contain at least one even and one
 * odd value, and ensures the result equals `a[firstEven] - a[firstOdd]` where these are the
 * earliest occurrences of each parity type.
 */

#include <stdbool.h>
#include <stddef.h>

/*@
  predicate is_even(integer n) = n % 2 == 0;
  predicate is_odd(integer n) = n % 2 != 0;
*/

/*@
  requires \valid(a + (0..n-1));
  requires n >= 2;
  requires \exists integer i; 0 <= i < n && is_even(a[i]);
  requires \exists integer i; 0 <= i < n && is_odd(a[i]);
  assigns \nothing;
  ensures \exists integer e, o; 
          0 <= e < n && 0 <= o < n && 
          is_even(a[e]) && is_odd(a[o]) &&
          (\forall integer k; 0 <= k < e ==> is_odd(a[k])) &&
          (\forall integer k; 0 <= k < o ==> is_even(a[k])) &&
          \result == a[e] - a[o];
*/
int FirstEvenOddDifference(const int a[], size_t n) {
    int firstEven = -1;
    int firstOdd = -1;

    /*@
      loop invariant 0 <= i <= n;
      loop invariant firstEven == -1 || 
        (0 <= firstEven < i && is_even(a[firstEven]) &&
         (\forall integer k; 0 <= k < firstEven ==> is_odd(a[k])));
      loop invariant firstOdd == -1 || 
        (0 <= firstOdd < i && is_odd(a[firstOdd]) &&
         (\forall integer k; 0 <= k < firstOdd ==> is_even(a[k])));
      loop assigns i, firstEven, firstOdd;
      loop variant n - i;
    */
    for (size_t i = 0; i < n; ++i) {
        if (firstEven == -1 && a[i] % 2 == 0) {
            firstEven = i;
        }
        if (firstOdd == -1 && a[i] % 2 != 0) {
            firstOdd = i;
        }
        if (firstEven != -1 && firstOdd != -1) {
            break;
        }
    }

    return a[firstEven] - a[firstOdd];
}