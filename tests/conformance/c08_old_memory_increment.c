/* rule: \old on memory cells after in-place update */
/*@ requires 0 <= n <= 4;
    requires \valid(a + (0 .. n-1));
    requires \forall integer i; 0 <= i < n ==> a[i] < 2147483647;
    assigns a[0 .. n-1];
    ensures \forall integer i; 0 <= i < n ==> a[i] == \old(a[i]) + 1; */
void incr_all(int *a, int n)
{
  /*@ loop invariant 0 <= i <= n;
      loop invariant \forall integer j; 0 <= j < i ==> a[j] == \at(a[j], Pre) + 1;
      loop invariant \forall integer j; i <= j < n ==> a[j] == \at(a[j], Pre);
      loop assigns i, a[0 .. n-1]; loop variant n - i; */
  for (int i = 0; i < n; i++)
    a[i] = a[i] + 1;
}
