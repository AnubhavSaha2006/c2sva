/*
 * Task 455 Natural Language:
 * Write an ACSL-annotated C function that multiplies an integer x by 4 and returns the result. The
 * function requires that x is within the range [INT_MIN/4, INT_MAX/4] to prevent integer overflow.
 * The function ensures both the return value and the global variable e equal 4*x, and assigns only
 * the global variable e.
 */

#include <limits.h>

int e;

/*@ requires
    INT_MIN / 4 <= x <= INT_MAX / 4;
  ensures
    \result == 4 * x && e == 4 * x;
  assigns e;
*/
int f(int x) {
  int y = 4 * x;
  e = y;
  return y;
}

int main() {
  f(42);
  return 0;
}