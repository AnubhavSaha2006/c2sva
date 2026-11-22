/*
 * Task 80 Natural Language:
 * Write an ACSL-annotated C function that checks if all elements in array 'a' of size 'n' are
 * identical. The function returns true if all elements match the first element a[0], false
 * otherwise. Requires a non-empty valid array (n > 0).
 */

#include <stdbool.h>
#include <stddef.h>

/*@
  requires \valid(a + (0..n-1));
  requires n > 0;
  assigns \nothing;
  ensures \result <==> (\forall integer i; 0 <= i < n ==> a[i] == a[0]);
*/
bool HasOnlyOneDistinctElement(const int a[], size_t n) {
    if (n == 0) {
        return true;
    }

    int firstElement = a[0];
    bool result = true;

    /*@
      loop invariant 1 <= i <= n;
      loop invariant result <==> (\forall integer j; 0 <= j < i ==> a[j] == firstElement);
      loop assigns i, result;
      loop variant n - i;
    */
    for (size_t i = 1; i < n; ++i) {
        if (a[i] != firstElement) {
            result = false;
            break;
        }
    }

    return result;
}