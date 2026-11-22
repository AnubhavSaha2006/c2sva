/*
 * Task 501 Natural Language:
 * Write an ACSL-annotated C function that finds the minimum value in an integer array `t` of size
 * `n`. The function requires a valid, non-empty array (`n > 0`). It ensures the result is less
 * than or equal to every element in the array and that the result is an actual element from the
 * array.
 */

/*@
requires \valid_read(t+(0..n-1)) && n > 0;
ensures \forall integer i; 0<=i<n ==> \result <= t[i];
ensures \exists integer i; 0<=i<n && t[i] == \result;
assigns \nothing;
*/
int min(int t[], int n) {
	int res = t[0];
	/*@
	loop invariant 1 <= i <= n;
	loop invariant \forall integer j; 0 <= j < i ==> res <= t[j];
	loop invariant \exists integer j; 0 <= j < i && res == t[j];
	loop assigns res, i;
	loop variant n - i;
	*/
	for (int i = 1; i < n; i++)
		if (t[i] < res) res = t[i];
	return res;
}