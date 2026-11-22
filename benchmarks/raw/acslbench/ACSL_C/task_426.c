/*
 * Task 426 Natural Language:
 * Write an ACSL-annotated C function that calculates 2*(x+1) for a negative integer x where
 * -1073741824 < x < 0. The function ensures no overflow occurs and returns the computed result
 * without modifying any external state.
 */

/*@ axiomatic ellipsoids_proof_tactics {
  @   type ellipsoids_tactics = Intuition | Tactic2;
  @   predicate use_strategy (ellipsoids_tactics t);
  @ }
*/

/*@ requires x < 0 && x > -1073741824;
  @ ensures \result == 2 * (x + 1);
  @ assigns \nothing;
*/
int plus_one (int x) {
int y,z;

{
  /*@ assert x + 1 <= 1073741823; */
  y = x + 1;
}

{
  /*@ assert 2 * y <= 2147483647; */
  z = 2 * y;
}
return z;
}