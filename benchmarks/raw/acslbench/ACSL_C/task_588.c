/*
 * Task 588 Natural Language:
 * Write an ACSL-annotated C function that initializes all elements of a 2D array (represented as a
 * 1D block of memory) to the value 7. The function takes three parameters: the number of rows
 * `x0`, the number of columns `x1`, and a pointer `x2` to the array data. It requires that `x0`
 * and `x1` are between 1 and 99, the memory block for the array is valid, and all initial array
 * values are between 0 and 255. The function ensures that every element in the array is set to 7
 * after execution.
 */

#include <limits.h>
/*@
requires
    0 < x0 && x0 < 100 &&
    0 < x1 && x1 < 100 &&
    \valid(x2 + (0..(x0 * x1) - 1)) &&
    (\forall integer i; 0 <= i < (x0 * x1) ==> 0 <= x2[i] < 256);
assigns x2[0..(x0*x1)-1];
ensures \forall integer i; 0 <= i < (x0 * x1) ==> x2[i] == 7;
*/
void p(int  x0, int  x1, int  * x2) {
  /*@
  loop invariant 0<=x5<=x0;
  loop invariant (\forall integer i; 0 <= i < (x5 * x1) ==> x2[i] == 7);
  loop invariant (\forall integer i; (x5 * x1) <= i < (x0 * x1) ==> 0 <= x2[i] < 256);
  loop assigns x5, x2[0..(x0*x1)-1];
  loop variant x0-x5;
  */
  for(int x5=0; x5 < x0; x5++) {
    int x11 = x5 * x1;
    /*@
    loop invariant 0<=x13<=x1;
    loop invariant (x11==(x5*x1));
    loop invariant (\forall integer i; 0 <= i < (x5 * x1 + x13) ==> x2[i] == 7);
    loop invariant (\forall integer i; (x5 * x1 + x13) <= i < (x0 * x1) ==> (0 <= x2[i] < 256 || x2[i] == 7));
    loop assigns x13, x2[0..(x0*x1)-1];
    loop variant x1-x13;
    */
    for(int x13=0; x13 < x1; x13++) {
      int x42 = x11 + x13;
      int x43 = x2[x42];
      x2[x42] = 7;
      //@ assert x2[x42] == 7;
    }
  }
}
