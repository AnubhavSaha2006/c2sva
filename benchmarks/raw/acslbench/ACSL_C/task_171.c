/*
 * Task 171 Natural Language:
 * Write an ACSL-annotated C program containing two functions: `max_seq` and `max_idx`. The
 * `max_seq` function returns the maximum value in an unsigned integer array `t` of length `len`.
 * It returns -1 if the array is empty. The `max_idx` auxiliary function returns the index of the
 * maximum value in the non-empty array `t`. Both functions require the array to be valid and
 * accessible in the range 0 to len-1. The specifications ensure the functions correctly identify
 * the maximum value/index and handle edge cases like empty arrays.
 */




#include <limits.h>
int max_idx(unsigned int *t, int len);


/*@ requires \valid(t+(0..len-1));
  @ requires len >= 0;
  @ requires \forall integer n; 0 <= n < len ==> t[n] <= INT_MAX;
  @ assigns \nothing;
  @ behavior t_vide:
  @    assumes len == 0;
  @    ensures \result == -1;
  @ behavior t_max:
  @    assumes len > 0;
  @    ensures \forall integer n; 0 <= n < len ==> \result >= t[n];
  @    ensures \exists int e; 0 <= e < len && \result == t[e];
  @    ensures \result >= 0;
  @ complete behaviors;
  @ disjoint behaviors;
  @ */
int max_seq(unsigned int *t, int len) {
  if (len <= 0) return -1;
  return t[max_idx(t, len)];
}


/*@ requires \valid(t+(0..len-1));
  @ requires len > 0;
  @ assigns \nothing;
  @ ensures len == 0 ==> \result == 0;
  @ ensures \forall integer n; 0 <= n < len ==> t[\result] >= t[n];
  @ ensures 0 <= \result < len;
  @ */
int max_idx(unsigned int *t, int len) {
  int max = 0;
  /*@loop invariant 1 <= i <= len;
    @loop invariant \forall integer n; 0 <= n < i ==> t[max] >= t[n];
    @loop invariant 0 <= max < len;
    @loop assigns i, max;
    @loop variant len - i;
    @ */
  for(int i = 1; i < len; i++)
    if (t[max] < t[i])
      max = i;
  return max;
}
