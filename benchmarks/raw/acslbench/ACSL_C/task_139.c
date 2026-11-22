/*
 * Task 139 Natural Language:
 * Write an ACSL-annotated C function that partially sorts array `a` of size `n` to ensure the
 * element at position `k` becomes the maximum among the first `k+1` elements. The function
 * requires `n > 0`, `0 ≤ k < n`, and a valid array. It performs a partial bubble sort up to index
 * `k`.
 */

/*@
	requires n>0;
	requires 0<=k<n;
	requires\valid(a+ (0..n-1));
	ensures\forall integer i;
	0<= i<=k ==>a[k]>=a[i];
*/
void q2(int a[],int n,int k)
{
/*@
	loop invariant\forall integer i;
	0<= i<j ==> a[j]>=a[i];loop invariant 0<=j<=k;
	loop assigns j,a[0..j+1];
	loop variant k-j;
*/
	for (int j=0;j<k;j++)
	{
		if(a[j]>a[j+1])
		{
			int t=a[j];
			a[j]=a[j+1];
			a[j+1]=t;
		}
	}
}