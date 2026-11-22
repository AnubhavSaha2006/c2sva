/*@ requires a > -2147483648;
    assigns \nothing;
    ensures \result >= 0;
    ensures a >= 0 ==> \result == a;
    ensures a < 0 ==> \result == -a;
*/
int spec_abs(int a)
{
  return a < 0 ? -a : a;
}
