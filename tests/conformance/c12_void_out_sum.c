/* rule: void function with a result written through a single-object pointer; recursive Sum */
/*@ logic integer Sum{L}(int *a, integer n) = n <= 0 ? 0 : Sum(a, n - 1) + a[n - 1]; */
/*@ requires 0 <= n <= 4;
    requires \valid_read(a + (0 .. n-1));
    requires \valid(out);
    requires \separated(out, a + (0 .. n-1));
    requires \forall integer i; 0 <= i < n ==> -1000 <= a[i] <= 1000;
    assigns *out;
    ensures *out == Sum(a, n); */
void sum_out(const int *a, int n, int *out)
{
  int s = 0;
  /*@ loop invariant 0 <= i <= n;
      loop invariant s == Sum(a, i);
      loop invariant -1000 * i <= s <= 1000 * i;
      loop assigns i, s; loop variant n - i; */
  for (int i = 0; i < n; i++)
    s += a[i];
  *out = s;
}
