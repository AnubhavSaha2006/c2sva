/*
 * Task 447 Natural Language:
 * Write an ACSL-annotated C function that finds the maximum element in an integer array `a` of
 * size `n`. The function requires a non-empty array (`n > 0`) and ensures the returned value is
 * greater than or equal to every element in the array.
 */

/*@
  requires n > 0;
  requires \valid_read(a + (0..n-1));
  ensures \forall integer j; 0 <= j < n ==> \result >= a[j];
  assigns \nothing;
*/
int Max_element(int a[], int n)
{
    int max = a[0];
    int i = 0;
    /*@
  loop invariant \forall integer j; 0 <= j < i ==> max >= a[j];
  loop invariant 1 <= i <= n;
  loop assigns i, max;
  loop variant n - i;
*/
    for (int i = 1; i < n; i++)
    {
        if (a[i] > max)
        {
            max = a[i];
        }
    }

    return max;
}