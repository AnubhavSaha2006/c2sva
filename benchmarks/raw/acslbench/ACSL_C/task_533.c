/*
 * Task 533 Natural Language:
 * Write an ACSL-annotated C function that checks if an integer array `t` of size `n` is a
 * palindrome. The function requires a valid array of size `n > 0` and ensures the result is true
 * (1) if and only if for all indices `i` in the first half of the array, the element at `i` equals
 * the element at the symmetric position `n-i-1`. The function returns 0 if the array is not a
 * palindrome.
 */

// teste si un tableau est un palindrome
// ESOPE RESTE ELU PAR CETTE CRAPULE ET SE REPOSE
/*@
 requires \valid (t+(0..n-1));
 requires n > 0;
 assigns \nothing;
 ensures \result <==> (\forall integer i; 0 <= i < n/2 ==> t[i] == t[n-i-1]);
 */
int is_palindrome(int t[], int n)
{
	int mid = n/2;
	int i = 0;

	/*@
	 loop invariant 0 <= i <= mid;
	 loop invariant \forall integer j; 0 <= j < i ==> t[j] == t[n-1-j];
	 loop assigns i;
	 loop variant mid - i;
	 */
	while (i < mid && t[i] == t[n-i-1])
	{
		i++;
	}

	return i == mid;
}