/*
 * Task 515 Natural Language:
 * Write an ACSL-annotated C function that checks if an integer array `t` of size `n` is a
 * palindrome. The function requires a valid array of size `n > 0`. It returns 1 if the array is a
 * palindrome (reads the same forwards and backwards), and 0 otherwise.
 */

/*@
	requires \valid(t+(0..n-1)) && n>0;
	// assigns \nothing;
	ensures (\forall integer i; 0<=i<n ==> t[i] == t[n-i-1]) ==> \result==1;
	ensures (\exists integer i; 0<=i<n && t[i] != t[n-i-1]) ==> \result==0;
*/

int palindrome(int t[], int n){
	/*@
		loop invariant 0<=i<=n/2;
		loop invariant \forall integer j; 0<=j<i ==> t[j] == t[n - j - 1];
		loop assigns i;
		loop variant n/2-i;
	*/
	for (int i = 0; i < n / 2; i++) {
        if (t[i] != t[n - i - 1])
            return 0;
    }
    return 1;
}