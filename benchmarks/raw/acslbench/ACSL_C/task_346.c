/*
 * Task 346 Natural Language:
 * Write an ACSL-annotated C function that performs a single pass of a bubble sort on array `a` of
 * size `n`, up to index `k`. The function ensures that after execution, the element at position
 * `k` is the largest among the first `k+1` elements. The function requires that `n` is positive,
 * `k` is a valid index between 0 and `n-1`, and the array `a` is valid for indices 0 to `n-1`.
 */

/*@
	requires n>0;
	requires 0<=k<n;
	requires\valid(a+ (0..n-1));
	ensures\forall integer i;
	0<= i<=k ==>a[k]>=a[i];
*/
void bubbleinnerloop(int a[], int n, int k)
{
	/*@
	loop invariant\forall integer i;
	0<= i<j ==> a[j]>=a[i];loop invariant 0<=j<=k;
	loop assigns j,a[0..j+1];
	loop variant k-j;
*/
	for (int j = 0; j < k; j++)
	{
		if (a[j] > a[j + 1])
		{
			int temp = a[j];
			a[j] = a[j + 1];
			a[j + 1] = temp;
		}
	}
}
