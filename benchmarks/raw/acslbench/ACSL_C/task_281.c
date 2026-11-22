/*
 * Task 281 Natural Language:
 * Write an ACSL-annotated C program that defines and uses an axiomatic definition for the absolute
 * value function. The program includes three functions: an unimplemented `abs` function that
 * returns the absolute value of an integer, a `max` function that returns the greater of two
 * integers, and a `max_abs` function that returns the maximum of the absolute values of two
 * integers, ensuring a non-negative result.
 */

/*@ axiomatic abs {

  @   logic int abs(int x);

  @   axiom pos: \forall int x; x >= 0 ==> abs(x) == x;

  @   axiom neg: \forall int x; x <= 0 ==>  abs(x) == -x;

  @ }

*/



/*@ ensures \result == abs(x); 

  @ assigns \nothing;

*/

int abs(int x);



/*@ ensures (\result == x || \result == y)

  @       && \result >= x && \result >= y; 

  @ assigns \nothing;

*/

int max(int x, int y);



/*@ ensures \result >= 0;

  @ assigns \nothing;

*/

int max_abs(int x, int y) {

  x = abs(x);

  y = abs(y);

  return max(x, y);

}

