/*
 * Task 496 Natural Language:
 * Write an ACSL-annotated C function `f` that takes two integers `x` and `y` and returns 1 if `x`
 * is 0 OR `y` is 1; otherwise, it returns 0. The function ensures the result is 1 under this
 * condition and 0 otherwise, and assigns no memory.
 */

/*@ ensures (x==0||y==1)?\result==1:\result == 0; 
  assigns \nothing;
 */
int f(int x, int y) { return (x==0||y==1); }

/*@ assigns \nothing; */
int main() {
  int x = f(42,1);
  int y = f(0,36);
  return 0;
}