/*
 * Task 561 Natural Language:
 * Write an ACSL-annotated C function that finds the minimum value in an integer array `arr` of
 * size `n`. The function requires a valid, non-empty array (`n > 0`) and ensures the result is
 * less than or equal to every element in the array.
 */

/*@
  requires n > 0;
  requires \valid_read(arr + (0..n-1));
  ensures \forall integer i;
      0 <= i < n ==> \result <= arr[i];
*/
int arraymin(int arr[], int n) {
int min = arr[0];
/*@
  loop invariant \forall integer i; 0 <= i < n ==> min <= arr[i] || i >= j;
  loop invariant 1 <= j <= n;
  loop assigns j, min;
  loop variant n - j;
*/
for (int j=1; j<n; j++)
{
   if (arr[j] < min)
      min = arr[j];
}

return min;

}