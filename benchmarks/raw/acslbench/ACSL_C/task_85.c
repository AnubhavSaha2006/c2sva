/*
 * Task 85 Natural Language:
 * Write an ACSL-annotated C function that finds the first odd element in array `a` of size `n`.
 * Returns the found odd value or -1 if no odds exist. Requires non-empty array access validity.
 */

#include <stdbool.h>
#include <stddef.h>

/*@ 
  requires \valid_read(a + (0..n-1));
  requires n > 0;
  assigns \nothing;
  ensures (\exists integer i; 0 <= i < n && a[i] % 2 != 0) ==> 
          (\exists integer k; 0 <= k < n && a[k] % 2 != 0 &&
           (\forall integer j; 0 <= j < k ==> a[j] % 2 == 0) && \result == a[k]);
  ensures (\forall integer i; 0 <= i < n ==> a[i] % 2 == 0) ==> \result == -1;
*/
int FindFirstOdd(const int a[], size_t n) {
    bool found = false;
    size_t index = 0;

    /*@
      loop invariant 0 <= index <= n;
      loop assigns index, found;
      loop variant n - index;
    */
    for (; index < n; index++) {
        if (a[index] % 2 != 0) {
            found = true;
            break;
        }
    }

    return found ? a[index] : -1;
}