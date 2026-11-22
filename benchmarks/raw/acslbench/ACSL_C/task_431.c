/*
 * Task 431 Natural Language:
 * Write an ACSL-annotated C function that checks if all elements in an integer array `t` of size
 * `n` are zero. The function requires a valid array of size `n > 0`. It returns 1 if all elements
 * are zero, and 0 if at least one non-zero element is found.
 */

/*@
requires \valid(t+(0..n-1)) && n>0;
ensures (\forall integer i; 0<=i<n ==> t[i] == 0) ==> \result == 1;
ensures (\exists integer i; 0<=i<n && t[i] != 0) ==> \result == 0;
*/

int all_zeros(int t[], int n){
	/*@
		loop invariant 0<=i<=n;
		loop invariant \forall integer j; 0 <= j < i ==> t[j] == 0;
		loop assigns i;
		loop variant n-i;
	*/
	for(int i=0;i<n;i++){
		if(t[i] != 0)
			return 0;
	}
	return 1;
}