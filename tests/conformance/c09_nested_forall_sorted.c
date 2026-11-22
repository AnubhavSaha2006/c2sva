/* rule: nested/multi-variable quantifiers in a postcondition, <==> */
/*@ requires 0 <= n <= 4;
    requires \valid_read(a + (0 .. n-1));
    assigns \nothing;
    ensures \result == 1 <==> (\forall integer i, j; 0 <= i <= j < n ==> a[i] <= a[j]);
    ensures \result == 0 || \result == 1; */
int is_sorted(const int *a, int n)
{
  /*@ loop invariant 1 <= i <= (n > 0 ? n : 1);
      loop invariant \forall integer k, l; 0 <= k <= l < i && l < n ==> a[k] <= a[l];
      loop assigns i; loop variant n - i; */
  for (int i = 1; i < n; i++)
    if (a[i - 1] > a[i]) return 0;
  return 1;
}
