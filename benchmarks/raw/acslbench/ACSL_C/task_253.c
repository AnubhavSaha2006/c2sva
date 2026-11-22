/*
 * Task 253 Natural Language:
 * Write an ACSL-annotated C function that computes the cumulative sum of an integer array in-
 * place. The function takes an array `arr` and its size `n` as parameters, requiring that `n > 0`
 * and the array is valid for reading in the range `[0, n-1]`. The function modifies the array such
 * that each element becomes the sum of all previous elements including itself.
 */

/*@
	requires n>0;
	requires\valid_read(arr + (0..n-1));
*/

void cummilativeSum(int arr[], int n)
{
	int sum = 0, prev = 0, j, x;

	/*@
		loop invariant \exists integer i; 0 <= i < n ==> arr[j] == prev + arr[i];
		loop invariant 0 <= j <= n;
		loop assigns j, arr[0..j+1], prev;
		loop variant n-j;
	*/

	for (j = 0; j < n; j++)
	{
		arr[j] = prev + arr[j];
		prev = arr[j];
	}
}
