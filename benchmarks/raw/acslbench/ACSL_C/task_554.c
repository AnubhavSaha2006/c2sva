/*
 * Task 554 Natural Language:
 * Write an ACSL-annotated C function that finds the index of the first occurrence of element `x`
 * in array `t` of size `n`. The function returns the index if `x` is found (a value between 0 and
 * n-1), otherwise it returns -1. It requires a valid array `t` of size `n > 0`.
 */

/*@
	requires \valid(t+(0..n-1)) && n>0;
	ensures (\exists integer i; 0<=i<n && t[i] == x) ==> -1<=\result<n;
	ensures (\forall integer i; 0<=i<n ==> t[i] != x) ==> \result == -1;
*/

int index(int t[], int n, int x){
	/*@
	loop invariant 0<=i<=n;
	loop invariant \forall integer j; 0<=j<i ==> t[j] != x;
	loop assigns i;
	loop variant n-i;
	*/
	for(int i=0;i<n;i++){
		if(t[i] == x)
			return i;
	}
	return -1;
}