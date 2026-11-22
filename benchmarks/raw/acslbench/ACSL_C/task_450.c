/*
 * Task 450 Natural Language:
 * Write an ACSL-annotated C program with two functions: `f` and `main`. Function `f` takes an
 * integer `x` (required to be ≤ 0) and returns it unchanged. The main function declares and
 * initializes structures of type `inner` and `outer`, setting `outer.inner.a` to 0. The program
 * includes structure definitions for `inner` (containing integers, char, and pointer) and `outer`
 * (containing inner structure pointer, instance, array, and primitive types).
 */


/*@ requires x <= 0;
    ensures  \result == x;
    assigns \nothing;
*/
int f(int x) { //@ assert x<=0;
  int y = x;
  /*@ assert y <= 0 &&
             x == y; */
  return y;
}

struct inner {
  int a, b;
  char c;
  void *p;
};

struct outer {
  struct inner *pinner;
  struct inner inner;
  int a;
  struct inner ainner[5];
  int b;
  char c;
  long long l;
};



int main (void)
{
  struct inner inner;
  struct outer outer;
  outer.inner.a = 0;
                         /*@ assert outer.inner.a == 0; */

  return 0;
}
