/*
 * Task 167 Natural Language:
 * Write an ACSL-annotated C function that partitions an array segment A[p..r] around a pivot
 * element (the last element A[r]). The function rearranges the array so that all elements less
 * than or equal to the pivot come before it, and all elements greater than the pivot come after
 * it. The function returns the final index of the pivot element. Requires valid array segment with
 * indices from p to r (0 ≤ p ≤ r).
 */

/*@ requires 0 <= p <= r && \valid(A+(p..r));
  @*/
int partition (int A[], int p, int r) 
{ 
  int x = A[r]; 
  int tmp, j, i = p-1; 

  /*@ loop invariant p <= j <= r && p-1 <= i < j;
    @ loop assigns tmp, i, j, A[p..r];
    @ loop variant r-j;
    @*/
  for(j=p; j<r; j++) 
    if (A[j] <= x) { 
      i++; 
      tmp = A[i];
      A[i] = A[j];
      A[j] = tmp;
    } 
  tmp = A[i+1];
  A[i+1] = A[r];
  A[r] = tmp;
  return i+1; 
}


