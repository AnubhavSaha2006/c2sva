/*
 * Task 493 Natural Language:
 * Write an ACSL-annotated C function that finds the index of the minimum element in an integer
 * array `t` of size `n`. The function requires a valid array of size `n > 0` and ensures the
 * returned index points to an element that is less than or equal to all other elements in the
 * array.
 */

/*@
requires \valid(t+(0..n-1)) && n > 0;
ensures \forall integer i; 0<=i<n ==> t[\result] <= t[i];
ensures \exists integer i; 0<=i<n && i == \result;
assigns \nothing;
*/
int min(int t[], int n) {
	int index = 0;
	/*@
	loop invariant 1 <= i <= n;
	loop invariant 0<=index<n;
	loop invariant \forall integer j; 0<=j<i ==> t[index] <= t[j];
        loop assigns index, i;
	loop variant n - i;
	*/
	for (int i = 1; i < n; i++){
		if (t[i] < t[index]) {
			index=i;
                  //@ assert \forall integer j; 0 <= j < i ==> t[index] <= t[j];
		}
	}
	return index;
}