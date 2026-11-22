/*
 * Task 488 Natural Language:
 * Write an ACSL-annotated C function that finds the minimum value in an integer array `a` of size
 * `n`. The function requires a valid, non-empty array (`n > 0`) and ensures the returned result is
 * less than or equal to every element in the array.
 */

/*@
  requires n > 0;
  requires \valid_read(a + (0..n-1));
  ensures \forall integer i; 0 <= i < n ==> \result <= a[i];
*/
int Min_elements(int a[], int n)
{
    int min = a[0];
/*@
  loop invariant 1 <= j <= n;
  loop invariant \forall integer k; 0 <= k < j ==> min <= a[k];
  loop assigns j, min;
  loop variant n - j;
*/
    for (int j = 1; j < n; j++)
    {
        if (a[j] < min)
            min = a[j];
    }

    return min;
}