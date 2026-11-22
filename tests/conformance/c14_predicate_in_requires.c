/* rule: user predicate in a precondition (universal assumption) and in a postcondition */
/*@ predicate InRange{L}(int *a, integer n, integer lo, integer hi) =
      \forall integer i; 0 <= i < n ==> lo <= a[i] <= hi; */
/*@ requires 0 <= n <= 4;
    requires \valid(a + (0 .. n-1));
    requires InRange(a, n, -100, 100);
    assigns a[0 .. n-1];
    ensures InRange(a, n, -200, 200);
    ensures \forall integer i; 0 <= i < n ==> a[i] == 2 * \old(a[i]); */
void double_all(int *a, int n)
{
  /*@ loop invariant 0 <= i <= n;
      loop invariant \forall integer j; 0 <= j < i ==> a[j] == 2 * \at(a[j], Pre);
      loop invariant \forall integer j; i <= j < n ==> a[j] == \at(a[j], Pre);
      loop assigns i, a[0 .. n-1]; loop variant n - i; */
  for (int i = 0; i < n; i++)
    a[i] = 2 * a[i];
}
