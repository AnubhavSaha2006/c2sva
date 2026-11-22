/* lower_bound from ACSL by Example (BinarySearch/lower_bound.{h,c}),
   contract, code and predicates copied verbatim; typedefs from typedefs.h. */

typedef int value_type;
typedef unsigned int size_type;

/*@
  predicate Increasing{L}(value_type* a, integer m, integer n) =
    \forall integer i, j; m <= i < j < n  ==>  a[i] <= a[j];

  predicate Increasing{L}(value_type* a, integer n) = Increasing{L}(a, 0, n);

  predicate LowerBound{L}(value_type* a, integer m, integer n, value_type v) =
    \forall integer i; m <= i < n  ==>  v <= a[i];

  predicate StrictUpperBound{L}(value_type* a, integer m, integer n, value_type v) =
    \forall integer i; m <= i < n  ==>  a[i] < v;
*/

/*@
  requires   valid:       \valid_read(a + (0..n-1));
  requires   increasing:  Increasing(a, n);

  terminates              \true;
  exits                   \false;
  assigns                 \nothing;

  ensures    result:      0 <= \result <= n;
  ensures    left:        StrictUpperBound(a, 0, \result, v);
  ensures    right:       LowerBound(a, \result, n, v);
*/
size_type lower_bound(const value_type* a, size_type n, value_type v)
{
  size_type left  = 0u;
  size_type right = n;

  /*@
    loop invariant bound:  0 <= left <= right <= n;
    loop invariant left:   StrictUpperBound(a, 0,  left, v);
    loop invariant right:  LowerBound(a, right, n, v);

    loop assigns left, right;
    loop variant right - left;
  */
  while (left < right) {
    const size_type middle = left + (right - left) / 2u;

    if (a[middle] < v) {
      left = middle + 1u;
    }
    else {
      right = middle;
    }
  }

  return left;
}
