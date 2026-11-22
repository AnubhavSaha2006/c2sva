/*
 * Task 449 Natural Language:
 * Write an ACSL-annotated C function that multiplies two integers, x and y, by repeatedly adding x
 * to a result variable y times. The function requires x to be between -10000 and 10000, and y to
 * be between 0 and 10000. It ensures the result equals the mathematical product x * y.
 */

/*@
  @ requires -10000 <= x <= 10000;
  @ requires      0 <= y <= 10000;
  @ 
  @ ensures \result == x * y;
  @ assigns \nothing;
*/
int mult(int x, int y) {

    int res = 0;

    /*@
      @ loop invariant 0 <= i <= y;
      @ loop invariant res == i * x;
      @ loop assigns res, i;
      @ loop   variant y - i;
    */
    for (int i = 0; i < y; ++i) {
        res += x;
    }

    return res;
}