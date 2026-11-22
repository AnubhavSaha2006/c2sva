/* rule: struct returned by value (C-aligned packing on ap_return) */
struct MinMax { int lo; int hi; };
/*@ assigns \nothing;
    ensures \result.lo <= \result.hi;
    ensures \result.lo == \min(a, b) && \result.hi == \max(a, b); */
struct MinMax order2(int a, int b)
{
  struct MinMax r;
  if (a <= b) { r.lo = a; r.hi = b; } else { r.lo = b; r.hi = a; }
  return r;
}
