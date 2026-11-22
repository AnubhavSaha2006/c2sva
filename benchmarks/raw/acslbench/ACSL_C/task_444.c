/*
 * Task 444 Natural Language:
 * Write an ACSL-annotated C function that uses a loop to transfer the value from variable `x` to
 * variable `y`. The function takes a positive integer `c` and ensures the return value equals `c`.
 * The loop maintains the invariant that the sum of `x` and `y` always equals the initial value
 * `c`.
 */

/*@
 requires c > 0;
 ensures \result == c;
 assigns \nothing;
*/
int loop(int c) {
 int x = c;
 int y = 0;
 /*@
 loop invariant c == x + y && x >= 0 ;
 loop assigns x, y;
 loop variant x;
 */
 while(x > 0) {
 x = x - 1;
 y = y + 1;
 }
 return y;
}

/*@ assigns \nothing; */
int main()
{
  int result = loop(6);
  return 0;
}