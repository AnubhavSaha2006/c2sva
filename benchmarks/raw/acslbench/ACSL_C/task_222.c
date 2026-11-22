/*
 * Task 222 Natural Language:
 * Write an ACSL-annotated C program with three functions. The first function, `pick_index`, takes
 * a positive integer `x0` and returns 0. The second function, `pick_element`, takes a valid array
 * `x9` of size `x10` (a positive integer) and returns the first element of the array. The third
 * function, `pick_first`, takes a valid pointer `x20` and returns the value it points to. All
 * functions have specific preconditions and ensure their results are valid without modifying any
 * external state.
 */

#include <limits.h>
/*@
requires (x0>0);
assigns \nothing;
ensures ((0<=\result) &&
(\result<x0));
*/
int pick_index(int  x0) {
  return 0;
}
/*@
requires ((x10>0) &&
\valid(x9+(0..x10-1)));
assigns \nothing;
*/
int pick_element(int  * x9, int  x10) {
  int x12 = pick_index(x10);
  int x13 = x9[x12];
  return x13;
}
/*@
requires \valid(x20);
assigns \nothing;
ensures (\result==x20[0]);
*/
int pick_first(int  * x20) {
  int x22 = x20[0];
  return x22;
}
