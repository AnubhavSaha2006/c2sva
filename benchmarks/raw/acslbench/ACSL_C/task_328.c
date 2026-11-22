/*
 * Task 328 Natural Language:
 * Write an ACSL-annotated C function that performs a binary search for element `x` in a sorted
 * array `a` of size `n`. The function requires the array to be sorted in non-decreasing order and
 * valid for reading. It returns the index of `x` if found, otherwise -1. The search maintains loop
 * invariants about the search boundaries and the relative position of `x` to the elements in the
 * excluded ranges.
 */

/*@ 

requires n > 0;

requires \forall integer i,j;  0 <= i <= j <= n-1   ==> a[i] <= a[j];

requires \valid_read(a + (0..n-1));

ensures (\forall integer k; 0 <= k <= n-1 ==> a[k] != x) || x == a[\result];

assigns \nothing;

*/

int binarysearch(int* a, int x, int n) {

  int low = -1;
  int high = n;
  int p;

  /*@ 
     loop invariant \forall integer i,j;  0 <= i <= j <= n-1   ==> a[i] <= a[j];
     loop invariant \forall integer k;  0 <= k <= low    ==> a[k] < x;
     loop invariant \forall integer k;  high <= k <= n-1 ==> a[k] > x;
     loop invariant -1 <= low+1 <= high <= n;

     loop assigns low, high, p;
 */

  while (low+1 < high) {
	p = (low + high) / 2;
	if (a[p] == x) 
		return p;
	else 
	    if (a[p] < x)
		 low = p;
	    else high = p;
  }
  return -1;
}
