/*
 * Task 156 Natural Language:
 * Write an ACSL-annotated C function that finds the first duplicate element in an array. The
 * function takes an array `a` of length `len` where all elements are between 0 and 99 inclusive.
 * It returns the index of the first duplicate element found, or -1 if no duplicates exist. The
 * specification uses behaviors for both duplicate-present and no-duplicate cases, with loop
 * invariants tracking seen values and ensuring no duplicates in processed elements.
 */



#define BOUND 100

/*@ predicate has_pair(int *a, integer len) =
  \exists integer i, j; 0 <= i < j < len && a[i] == a[j]; */

/*@ requires len >= 0;
  @ requires \valid(a+(0.. len-1));
  @ requires \forall integer i; 0 <= i < len ==> 0 <= a[i] < BOUND;
  @ assigns \nothing;
  @
  @ behavior no_pair:
  @   assumes ! has_pair(a, len);
  @   ensures \result == -1;
  @
  @ behavior has_pair:
  @   assumes has_pair(a, len);
  @   ensures \exists integer i; 
  @     0 <= i < len && i != \result && a[\result] == a[i];
  @
  @ complete behaviors;
  @ disjoint behaviors;
  @ */
int pair(int *a, int len)
{
  char seen[BOUND];

  /*@ loop invariant 0 <= i <= BOUND;
    @ loop invariant \forall integer j; 0 <= j < i ==> seen[j] == 0;
    @
    @ loop assigns i, seen[0.. BOUND-1];
    @ loop variant BOUND-i; */
  for(int i = 0; i < BOUND; i++)
    seen[i] = 0;

  /*@ loop invariant 0 <= i <= len;
    @
    @ loop invariant
    @   \forall integer v; 0 <= v < BOUND ==> seen[v]
    @   ==> \exists integer j; 0 <= j < i && a[j] == v;
    @
    @ loop invariant
    @   \forall integer v; 0 <= v < BOUND ==> ! seen[v]
    @   ==> \forall integer j; 0 <= j < i ==> a[j] != v;
    @
    @ loop invariant !has_pair(a, i);
    @
    @ loop assigns i, seen[0.. BOUND-1];
    @ loop variant len-i; */
  for(int i = 0; i < len; i++) {
    int v = a[i];
    if (seen[v]) return i;
    else seen[v] = 1;
  }

  return -1;
}
