/*
 * Task 419 Natural Language:
 * Write an ACSL-annotated C function that checks if there exist two distinct indices i and j in an
 * integer array such that the sum of the elements at these indices equals a given target value X.
 * The function requires a valid array of size N > 0 with all elements between -1,000,000,000 and
 * 1,000,000,000, and ensures the result is true if and only if such a pair exists.
 */

#include <stdbool.h>

/*@ requires N > 0;
  @ requires \valid(a+(0..N-1));
  @ requires \forall integer i;
  @            0 <= i < N ==> -1000000000 <= a[i] <= 1000000000; 
  @ ensures  \result <==> (\exists integer i, j;
  @                        0 <= i < j < N && a[i] + a[j] == X);
  @*/
bool sumx (int N, int a[], int X)
{
  int i, j;
  /*@ loop invariant 0 <= i < N;
    @ loop invariant \forall integer ii, jj;
    @                  0 <= ii < jj < N && ii < i ==> a[ii] + a[jj] != X;
    @ loop assigns i, j;
    @ loop variant N-i-2;
    @*/
  for (i=0; i<N-1; i++)
    /*@ loop invariant i+1 <= j <= N;
      @ loop invariant \forall integer ii, jj;
      @                  0 <= ii < jj < N && ii < i ==> a[ii] + a[jj] != X;
      @ loop invariant \forall integer jj;
      @                  0 <= i < jj < j ==> a[i] + a[jj] != X;
      @ loop assigns j;
      @ loop variant N-j;
      @*/
    for (j=i+1; j<N; j++)
      if (a[i] + a[j] == X)
        return true;
  return false;
}
