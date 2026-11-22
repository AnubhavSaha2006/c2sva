/* rule: typedef'd narrow types, three arrays, ranged assigns (frame), unsigned wraparound */
typedef unsigned char u8;
/*@ requires 0 <= n <= 4;
    requires \valid_read(a + (0 .. n-1)) && \valid_read(b + (0 .. n-1));
    requires \valid(c + (0 .. n-1));
    requires \separated(c + (0 .. n-1), a + (0 .. n-1), b + (0 .. n-1));
    assigns c[0 .. n-1];
    ensures \forall integer i; 0 <= i < n ==> c[i] == (u8)(a[i] + b[i]); */
void add_vec(const u8 *a, const u8 *b, u8 *c, int n)
{
  /*@ loop invariant 0 <= i <= n;
      loop invariant \forall integer j; 0 <= j < i ==> c[j] == (u8)(a[j] + b[j]);
      loop assigns i, c[0 .. n-1]; loop variant n - i; */
  for (int i = 0; i < n; i++)
    c[i] = (u8)(a[i] + b[i]);
}
