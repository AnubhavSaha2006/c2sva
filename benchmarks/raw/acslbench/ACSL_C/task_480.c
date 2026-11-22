/*
 * Task 480 Natural Language:
 * Write an ACSL-annotated C function that swaps the elements at indices i and j in integer array a
 * of size n. The function requires valid array indices (0 ≤ i < n and 0 ≤ j < n) and ensures that
 * after execution, a[i] contains the original value of a[j] and a[j] contains the original value
 * of a[i].
 */

/*@ requires \valid(a+(0..n-1));
    requires 0 <= i < n;
    requires 0 <= j < n;
    assigns a[i], a[j];
    ensures a[i] == \old(a[j]);
    ensures a[j] == \old(a[i]);
 */
static inline void swap(int a[], int i, int j, int n)
{
  int tmp = a[i];
  a[i] = a[j];
  a[j] = tmp;
}