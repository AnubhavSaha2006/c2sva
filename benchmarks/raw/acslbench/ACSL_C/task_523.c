/*
 * Task 523 Natural Language:
 * Write an ACSL-annotated C function that searches for an element `x` in an array `a` of size `n`.
 * The function returns 1 if `x` is found in the array and 0 otherwise. The function requires that
 * the array size `n` is greater than 0 and that the memory range for the array is valid for
 * reading. The two behaviors `present` and `not_present` are complete and disjoint, covering all
 * possible cases.
 */

/*@ requires n > 0;
    requires \valid_read(a + (0..n-1));
    assigns \nothing;

    behavior present:
        assumes \exists integer k; 0 <= k < n && x == a[k];
        ensures \result == 1;
        assigns \nothing;

    behavior not_present:
        assumes \forall integer k; 0 <= k < n ==> x != a[k];
        ensures \result == 0;
        assigns \nothing;

    complete behaviors present, not_present;
    disjoint behaviors present, not_present;
*/
int arraysearch(int* a, int x, int n) {
  /*@ loop invariant 0 <= p <= n;
      loop invariant \forall integer k; 0 <= k < p ==> x != a[k];
      loop assigns p;
      loop variant n - p;
  */
  for (int p = 0; p < n; p++) {
    if (x == a[p])
       return 1;
  }
  return 0;
}