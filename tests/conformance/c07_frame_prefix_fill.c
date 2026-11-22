/* rule: partial frame - assigns a[0..k-1] only; cells k..n-1 must keep their values */
/*@ requires 0 <= k <= n <= 4;
    requires \valid(a + (0 .. n-1));
    assigns a[0 .. k-1];
    ensures \forall integer i; 0 <= i < k ==> a[i] == v; */
void fill_prefix(int *a, int n, int k, int v)
{
  /*@ loop invariant 0 <= i <= k;
      loop invariant \forall integer j; 0 <= j < i ==> a[j] == v;
      loop assigns i, a[0 .. k-1]; loop variant k - i; */
  for (int i = 0; i < k; i++)
    a[i] = v;
}
