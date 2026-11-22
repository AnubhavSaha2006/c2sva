/*
 * Task 468 Natural Language:
 * Write an ACSL-annotated C program with three functions: `f`, `g`, and `main`. Function `f`
 * requires a valid pointer `p` to an integer with value 0 and ensures it returns 0. Function `g`
 * requires a valid pointer `p` to an integer with value 1 and ensures it returns 0. Both `f` and
 * `g` contain a loop that iterates once and assign nothing. The `main` function initializes two
 * integers, calls `f` with a pointer to 0, and `g` with a pointer to 1.
 */

/

/*@ requires \valid(p);
  @ requires *p == 0;
  @ ensures \result == 0;
  @ assigns \nothing; */
int f(int *p) {
  /*@ loop invariant 0 <= i <= 1;
      loop assigns i;
      loop variant 1 - i; */
  for(int i = 0; i < 1; i++) ;
  return 0;
}

/*@ requires \valid(p);
  @ requires *p == 1;
  @ ensures \result == 0;
  @ assigns \nothing; */
int g(int *p) {
  /*@ loop invariant 0 <= i <= 1;
      loop assigns i;
      loop variant 1 - i; */
  for(int i = 0; i < 1; i++) ;
  return 0;
}

/*@ assigns \nothing; */
int main(void) {
  int x = 0;
  int y = 1;
  f(&x);
  g(&y);
}