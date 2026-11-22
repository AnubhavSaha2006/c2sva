/*
 * Task 46 Natural Language:
 * Write an ACSL-annotated C function that finds the longest IntArray (containing elements and
 * length) from an array of IntArray structures. The function requires valid non-empty input lists
 * and ensures the returned list has maximum length among all input lists. Parameters are lists
 * (array of IntArray) and list_count (number of lists).
 */

#include <stdlib.h>
#include <assert.h>

/*@
  requires lists != NULL && list_count > 0;
  requires \valid(lists+(0..list_count-1));
  requires \forall integer i; 0 <= i < list_count ==> \valid(lists[i].elements+(0..lists[i].length-1));
  assigns \nothing;
  ensures \forall integer i; 0 <= i < list_count ==> lists[i].length <= \result.length;
  ensures \exists integer i; 0 <= i < list_count && lists[i].length == \result.length;
*/
struct IntArray {
    int* elements;
    size_t length;
};

struct IntArray MaxLengthList(const struct IntArray* lists, size_t list_count) {
    struct IntArray maxList = lists[0];

    /*@
      loop invariant 1 <= i <= list_count;
      loop invariant \forall integer j; 0 <= j < i ==> lists[j].length <= maxList.length;
      loop invariant \exists integer j; 0 <= j < i && lists[j].length == maxList.length;
      loop assigns i, maxList;
      loop variant list_count - i;
    */
    for (size_t i = 1; i < list_count; ++i) {
        if (lists[i].length > maxList.length) {
            maxList = lists[i];
        }
    }

    return maxList;
}