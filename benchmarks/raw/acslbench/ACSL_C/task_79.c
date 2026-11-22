/*
 * Task 79 Natural Language:
 * Write an ACSL-annotated C function that checks if array `a` of size `n` forms a valid min-heap.
 * The function verifies each parent node is less than or equal to its children, returning true if
 * the heap property holds for all elements. Requires non-empty array (n > 0).
 */

#include <stdbool.h>
#include <assert.h>

/*@
  requires n > 0;
  assigns \nothing;
  ensures \result == (\forall integer i; 0 <= i < n/2 ==> 
                     (a[i] <= a[2*i+1] && (2*i+2 < n ==> a[i] <= a[2*i+2])));
*/
bool IsMinHeap(const int a[], unsigned int n) {
    bool result = true;
    unsigned int half = n / 2;

    /*@
      loop invariant 0 <= i <= half;
      loop invariant result == (\forall integer j; 0 <= j < i ==>
                              (a[j] <= a[2*j+1] && (2*j+2 < n ==> a[j] <= a[2*j+2])));
      loop assigns i, result;
      loop variant half - i;
    */
    for (unsigned int i = 0; i < half; ++i) {
        if (a[i] > a[2*i+1] || (2*i+2 < n && a[i] > a[2*i+2])) {
            result = false;
            break;
        }
    }

    return result;
}