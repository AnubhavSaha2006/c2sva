/* rule: array of structs (ap_memory, packed words), quantified field property */
struct Item { int key; unsigned char tag; };
/*@ requires 0 <= n <= 4;
    requires \valid_read(a + (0 .. n-1));
    assigns \nothing;
    ensures -1 <= \result < n;
    ensures \result >= 0 ==> a[\result].tag == t;
    ensures \result == -1 ==> (\forall integer i; 0 <= i < n ==> a[i].tag != t); */
int find_tag(const struct Item *a, int n, unsigned char t)
{
  /*@ loop invariant 0 <= i <= n;
      loop invariant \forall integer j; 0 <= j < i ==> a[j].tag != t;
      loop assigns i; loop variant n - i; */
  for (int i = 0; i < n; i++)
    if (a[i].tag == t) return i;
  return -1;
}
