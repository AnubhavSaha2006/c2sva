/* rule: \exists in a postcondition, <==> (both polarities), read-only array */
/*@ requires 0 <= n <= 4;
    requires \valid_read(a + (0 .. n-1));
    assigns \nothing;
    ensures \result == 1 <==> (\exists integer i; 0 <= i < n && a[i] == 0);
    ensures \result == 0 || \result == 1; */
int has_zero(const int *a, int n)
{
  /*@ loop invariant 0 <= i <= n;
      loop invariant \forall integer j; 0 <= j < i ==> a[j] != 0;
      loop assigns i; loop variant n - i; */
  for (int i = 0; i < n; i++)
    if (a[i] == 0) return 1;
  return 0;
}
