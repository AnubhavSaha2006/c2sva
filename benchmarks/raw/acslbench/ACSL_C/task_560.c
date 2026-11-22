/*
 * Task 560 Natural Language:
 * Write an ACSL-annotated C function that checks if an integer array is monotonically increasing.
 * The function takes an array `t` and its size `taille` as input, requires a valid non-empty
 * array, and returns 1 if the array is monotonically increasing (each element is less than or
 * equal to the next), otherwise returns 0.
 */

/*@
requires \valid(t+(0..taille-1)) && taille>0;
assigns \nothing;

behavior monotonic_increasing:
  assumes \forall integer i; 0 <= i < taille - 1 ==> t[i] <= t[i+1];
  ensures \result == 1;

behavior not_monotonic_increasing:
  assumes \exists integer i; 0 <= i < taille - 1 && t[i] > t[i+1];
  ensures \result == 0;

complete behaviors monotonic_increasing, not_monotonic_increasing;
disjoint behaviors monotonic_increasing, not_monotonic_increasing;
*/
int monotonic(int t[], int taille) {
  /*@
    loop invariant 1 <= i <= taille;
    loop invariant \forall integer j; 0 <= j < i-1 ==> t[j] <= t[j+1];
    loop assigns i;
    loop variant taille - i;
  */
  for (int i = 1; i < taille; i++) {
    if (t[i] < t[i - 1])
      return 0;
  }
  return 1;
}
