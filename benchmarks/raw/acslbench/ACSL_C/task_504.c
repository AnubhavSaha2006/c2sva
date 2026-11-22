/*
 * Task 504 Natural Language:
 * Write an ACSL-annotated C function that compares two integer arrays `s` and `t` of size `n`. The
 * function returns 1 (true) if all elements in both arrays are equal, and 0 (false) otherwise. It
 * requires that both arrays are valid for indices 0 to n-1 and that n is greater than 0.
 */

/*@
 requires \valid (s+(0..n-1));
 requires \valid (t+(0..n-1));
 requires n > 0;
 assigns \nothing;
 ensures \result <==> (\forall integer i; 0 <= i < n ==> s[i] == t[i]);
 */
int compare_array(int s[], int t[], int n)
{
	int i = 0;

	/*@
	 loop invariant \forall integer j; 0 <= j < i ==> s[j] == t[j];
	 loop invariant 0 <= i <= n;
	 loop assigns i;
	 loop variant n-i;
	 */
	while (i < n && s[i] == t[i])
	{
		i++;
	}

	return i == n;
}