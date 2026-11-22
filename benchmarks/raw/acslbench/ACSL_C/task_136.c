/*
 * Task 136 Natural Language:
 * Write an ACSL-annotated C program with three functions: `pick_index` returns a valid index
 * between 0 and n-1 for a given positive n; `pick_element` uses `pick_index` to safely access an
 * element from a valid array p of size n; and `pick_first` returns the first element of a valid
 * array p.
 */

/*@ requires n > 0;
    assigns \nothing;
    ensures 0 <= \result && \result < n;
*/
int pick_index(int n) { return 0; }

/*@ requires n > 0 && \valid(p+ (0..n-1));
    assigns \nothing;
*/
int pick_element(int* p, int n) {
  int i = pick_index(n);
  //@ assert (0 <= i && i < n);
  return p[i];
}

/*@
  requires \valid(p);
  assigns \nothing;
  ensures \result == p[0];
*/
int pick_first(int* p) {
  return p[0];
}
