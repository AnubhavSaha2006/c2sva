#define N 4

/*@ requires 0 < n <= N;
    requires \valid_read(a + (0..n-1));
    assigns \nothing;
    ensures 0 <= \result < n;
    ensures \forall integer i; 0 <= i < n ==> a[i] <= a[\result];
*/
unsigned max_element(const int a[N], unsigned n)
{
  unsigned max = 0;
  /*@ loop invariant 1 <= i <= n;
      loop invariant 0 <= max < i;
      loop invariant \forall integer j; 0 <= j < i ==> a[j] <= a[max];
      loop assigns i, max;
      loop variant n - i;
  */
  for (unsigned i = 1; i < n; i++) {
#pragma HLS LOOP_TRIPCOUNT max=4
    if (a[max] < a[i]) max = i;
  }
  return max;
}
