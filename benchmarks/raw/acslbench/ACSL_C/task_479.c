/*
 * Task 479 Natural Language:
 * Write an ACSL-annotated C function that initializes all elements of an integer array to zero.
 * The function takes an array `a` and its size `n` as parameters, requires that `n` is positive
 * and the array is valid, and ensures that every element in the array becomes zero after
 * execution.
 */

/*@
  requires n > 0;
  requires \valid(a + (0..n-1));
  ensures \forall integer i;
      0 <= i < n ==> a[i]==0;
*/
void Zero_array(int a[], int n)
{
/*@
  loop invariant \forall integer i;
      0 <= i < j ==> a[i]==0;
  loop invariant 0 <= j <= n;
  loop assigns j,a[0..n-1];
  loop variant n - j;
*/
    for (int j = 0; j < n; j++)
    {
        a[j] = 0;
    }
}