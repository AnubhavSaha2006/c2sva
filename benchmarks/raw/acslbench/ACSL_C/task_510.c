/*
 * Task 510 Natural Language:
 * Write an ACSL-annotated C function that finds the maximum value in an integer array. The
 * function takes an array `arr` and its size `n` as parameters, requires that `n > 0` and the
 * array is valid for reading, and ensures the result is greater than or equal to every element in
 * the array.
 */

/*@
  requires n > 0;
  requires \valid_read(arr + (0..n-1));
  ensures \forall integer i; 0 <= i < n ==> \result >= arr[i];
*/
int arraymax(int arr[], int n) {
int max = arr[0];
/*@
  loop invariant 1 <= j <= n;
  loop invariant \forall integer i; 0 <= i < j ==> max >= arr[i];
  loop invariant max >= arr[0];
  loop assigns j, max;
  loop variant n - j;
*/
for (int j=1; j<n; j++)
{
   if (arr[j] > max)
      max = arr[j];
}

return max;

}