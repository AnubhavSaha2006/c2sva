/*
 * Task 451 Natural Language:
 * Write an ACSL-annotated C function that initializes an integer to 0, increments it by 1, and
 * returns the result. The function ensures the return value is positive.
 */

/*@ ensures (\result>0); */
int main() {
  int x0 = 0;
  /*@assert (x0<100);*/
  int x4 = x0 + 1;
  /*@assert (x4>x0);*/
  return x4;
}