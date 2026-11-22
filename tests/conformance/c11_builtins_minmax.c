/* rule: ACSL builtins \max, \min, \abs */
/*@ requires a > -2147483648 && b > -2147483648;
    assigns \nothing;
    ensures \result == \max(\abs(a), \abs(b)); */
int max_abs(int a, int b)
{
  int x = a < 0 ? -a : a;
  int y = b < 0 ? -b : b;
  return x > y ? x : y;
}
