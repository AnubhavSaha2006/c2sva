/*
 * Task 555 Natural Language:
 * Write an ACSL-annotated C function that performs a binary search on a sorted array `a` of size
 * `n` for a given `key`. The function requires the array to be sorted in non-decreasing order and
 * valid for indices 0 to n-1. It returns a pointer to the found element if the key exists in the
 * array, or NULL if the key does not exist. The function must handle both cases where the key is
 * present and absent, ensuring complete and disjoint behavior coverage.
 */

#include <stdlib.h>

/*@ requires n >= 0;
    requires \valid(a+(0..n-1));
    requires \forall integer i, j; 0 <= i < j < n ==> a[i] <= a[j];
    assigns \result \from a[0..n-1];
    behavior NOT_EXISTS:
       assumes \forall integer i; 0 <= i < n ==> a[i] != key;
       ensures \result == \null;
    behavior EXISTS:
       assumes \exists integer i; 0 <= i < n && a[i] == key;
       ensures \exists integer i; 0 <= i < n && \result == (a + i) && \result >= a && \result < a + n;
       ensures \result != \null ==> *\result == key;
    complete behaviors NOT_EXISTS, EXISTS;
    disjoint behaviors NOT_EXISTS, EXISTS;
 */
int *spec_bsearch(int a[], int n, int key)
{
	int left = 0;
	int right = n - 1;

	/*@ loop invariant 0 <= left <= n;
	    loop invariant -1 <= right < n;
	    loop invariant \forall integer i; 0 <= i < n && a[i] == key ==> left <= i <= right+1;
            loop invariant (\exists integer i; 0 <= i < n && a[i] == key) ==> (\exists integer i; left <= i <= right && a[i] == key);
	    loop invariant right - left + 1 >= 0;
	    loop assigns left, right;
	    loop variant right - left + 1;
	 */
	while (left <= right) {
		int m = left + (right - left) / 2;

		if (a[m] < key) {
			left = m + 1;
		} else if (a[m] > key) {
			right = m - 1;
		} else {
			return &a[m];
		}
	}

	return NULL;
}
