/* rule: behaviors (assumes/ensures), complete/disjoint, scalar result */
/*@ requires lo <= hi;
    assigns \nothing;
    behavior below: assumes v < lo;  ensures \result == lo;
    behavior above: assumes v > hi;  ensures \result == hi;
    behavior inside: assumes lo <= v <= hi; ensures \result == v;
    complete behaviors; disjoint behaviors; */
int clamp(int v, int lo, int hi)
{
  if (v < lo) return lo;
  if (v > hi) return hi;
  return v;
}
