/*
 * Task 460 Natural Language:
 * Write an ACSL-annotated C function that increments the value at pointer `a` by the value at
 * pointer `b`, ensuring no integer overflow occurs. The function requires valid pointers and
 * checks both positive and negative addition scenarios to prevent underflow/overflow. The function
 * modifies only the value at `a` and ensures the result equals the sum of the original values.
 */



#include <limits.h>

/*@
  requires (*b >= 0 && *a <= INT_MAX - *b);
  requires (*b < 0 && *a >= INT_MIN - *b);
  requires \valid(a) && \valid(a);
  assigns *a;
  ensures *a == \old(*a) + *b;
*/
void incr_a_by_b(int* a, int const* b){
  *a += *b;
}
