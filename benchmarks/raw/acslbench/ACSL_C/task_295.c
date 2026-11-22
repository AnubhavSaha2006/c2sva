/*
 * Task 295 Natural Language:
 * Write an ACSL-annotated C function that counts how many buildings in a skyline array `a` of size
 * `N` can see the sunset to the west. The function requires a valid array of size N > 0 and
 * ensures the result equals the number of buildings taller than all buildings to their right. The
 * array is traversed from right to left, tracking the current maximum height to determine
 * visibility.
 */


/*@ predicate canSee{L}(integer N, int *x, integer i) =
  @   \forall integer j; i < j < N ==> \at(x[i], L) > \at(x[j], L);
  @*/

/*@ predicate countRangeFrom{L}(integer N, int *x, integer i, integer c) =
  @   i >= N ? c == 0 :
  @   canSee{L}(N, x, i) ? countRangeFrom{L}(N, x, i+1, c-1)
  @                      : countRangeFrom{L}(N, x, i+1, c);
  @*/

/*@ requires N > 0;
  @ requires \valid(a + (0..N-1));
  @ ensures  countRangeFrom(N, a, 0, \result);
  @*/
int count(int N, int* a) {
  int max = a[N-1];
  int result = 1;
  /*@ loop invariant -1 <= i <= N-2;
    @ loop invariant result + i <= N-1;
    @ loop invariant \exists integer j; i < j < N && a[j] == max;
    @ loop invariant \forall integer j; i < j < N ==> a[j] <= max;
    @ loop invariant countRangeFrom(N, a, i+1, result);
    @ loop assigns i, max, result;
    @ loop variant i+1;
    @*/
  for (int i = N-2; i >= 0; --i)
    if (a[i] > max) {
      max = a[i];
      ++result;
    }
  return result;
}