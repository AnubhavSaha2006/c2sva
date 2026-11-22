/*
 * Task 274 Natural Language:
 * Write an ACSL-annotated C function that fills an integer array with a given value. The function
 * takes an array `t` of size `n` and a value `val`, and assigns `val` to every element in the
 * array. Requires that `n` is greater than 0 and that the array is valid for indices 0 to n-1.
 */


/*@
 requires \valid (t+(0..n-1));
 requires n > 0;
 assigns t[0..n-1];
 ensures \forall integer i; 0 <= i < n ==> t[i] == val;
 */
void fill_array(int t[], int n, int val)
{

	int i = 0;

	/*@
	 loop invariant \forall integer j; 0 <= j < i ==> t[j] == val;
	 loop invariant 0 <= i <= n;
	 loop variant n-i;
	 */
	while (i < n)
	{
		t[i] = val;
		i++;
	}
}
