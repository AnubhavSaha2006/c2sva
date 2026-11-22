/*
 * Task 459 Natural Language:
 * Write an ACSL-annotated C function that resets all elements of an integer array to zero. The
 * function requires a valid array `arr` of size `n` (where `n > 0`) and ensures that every element
 * in the array is zero after execution.
 */

/*@
  requires n > 0;
  requires \valid(arr+(0..n-1));
  ensures \forall integer i; 0 <= i < n ==> arr[i] == 0;
*/
void arrayreset(int arr[], int n) {

/*@
  loop invariant 0 <= j <= n;
  loop invariant \forall integer i; 0 <= i < j ==> arr[i] == 0;
  loop assigns j, arr[0..n-1];
  loop variant n - j;
*/
for (int j=0; j<n; j++)
{
   arr[j]=0;
}


}