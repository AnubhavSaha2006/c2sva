/*
 * Task 140 Natural Language:
 * Write an ACSL-annotated C function that processes an array `a` of size `n` to ensure the last
 * element `a[n-1]` becomes the largest element in the array. The function requires `n > 0` and a
 * valid array. It performs a single pass through the array, swapping adjacent elements when
 * necessary to bubble larger values toward the end.
 */

/*@
	requires n>0;
	requires\valid(a+ (0..n-1));
	ensures\forall integer i;
	0<= i<n-1 ==>a[n-1]>=a[i];
*/
void bubbleinnerloop(int a[], int n)
{
	/*@
	loop invariant\forall integer i;
	0<= i<j ==> a[j]>=a[i];loop invariant 0<=j<=n-1;
	loop assigns j,a[0..j+1];
	loop variant n-1-j;
*/
	for (int j = 0; j < n - 1; j++)
	{
		if (a[j] > a[j + 1])
		{
			int temp = a[j];
			a[j] = a[j + 1];
			a[j + 1] = temp;
		}
	}
}
