/*
 * Task 465 Natural Language:
 * Write an ACSL-annotated C program with three functions: `f`, `g`, and `main`. Function `f`
 * assigns a global variable `x` to the input `i`, requiring `i >= -1`. Function `g` increments `x`
 * by 1 if `x < 8`, otherwise leaves it unchanged, requiring `x <= 8`. The `main` function
 * conditionally calls `f` with different arguments based on input `c`, then calls `g` twice,
 * ensuring `x` remains less than or equal to 8 throughout.
 */


// Fuse with precond.c when bts #1208 is solved
int x = 0;

/*@ requires i >= -1;
  assigns x;
  ensures x == i;
 */
void f (int i) {
  x = i;
}

/*@ requires x <= 8;
  assigns x;
  ensures x == \old(x) + 1 || x == \old(x);
 */
void g();

void g() {
  if (x < 8) {
    x++;
  }
}

/*@ requires -1 <= 8; // Ensure x <= 8 after the calls to f in the if(c) blocks.
    requires x <= 8;
    assigns x;
*/
void main (int c) {
  if (c) {
    f(1);
    if(c) f(-1);
  }
  //@ assert x <= 8;
  g ();
  //@ assert x <= 8;
  g ();
}