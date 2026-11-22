/*
 * Task 598 Natural Language:
 * Write an ACSL-annotated C function that multiplies two integers x and y using repeated addition.
 * The function requires that both x and y are in the range (0, 100] and ensures the result equals
 * x*y, is in the range (0, 10000], and no external memory is modified.
 */

/*@ requires 0<x<=100 && 0<y<=100;
    ensures \result == x*y;
    ensures 0<\result<=10000;
    assigns \nothing;
*/


int mul(int x, int y){
  int mul=0;
  /*@
  	loop invariant 0<=i<=y;
  	loop invariant mul==x*i;
    loop assigns mul,i;
    loop variant y-i;
  */
  for(int i=0;i<y;i++)
    mul+=x;
  return mul;
}