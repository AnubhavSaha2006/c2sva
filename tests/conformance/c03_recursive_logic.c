/* rule: user-defined recursive logic function (bounded unrolling) */
/*@ logic integer CountPos{L}(int *a, integer n) =
      n <= 0 ? 0 : CountPos(a, n - 1) + (a[n - 1] > 0 ? 1 : 0); */
/*@ requires 0 <= n <= 4;
    requires \valid_read(a + (0 .. n-1));
    assigns \nothing;
    ensures \result == CountPos(a, n); */
int count_pos(const int *a, int n)
{
  int c = 0;
  /*@ loop invariant 0 <= i <= n;
      loop invariant c == CountPos(a, i);
      loop assigns i, c; loop variant n - i; */
  for (int i = 0; i < n; i++)
    if (a[i] > 0) c++;
  return c;
}
