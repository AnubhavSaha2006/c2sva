/*
 * Task 370 Natural Language:
 * Write an ACSL-annotated C function that checks if a quadratic equation has real roots. The
 * function takes three integer coefficients a, b, c and returns 0 if the discriminant (b² - 4ac)
 * is non-negative (real roots exist), or 1 if the discriminant is negative (no real roots).
 */

/*@
ensures (((((b*b)-4*a*c)>=0)&&(\result==0))||((((b*b)-4*a*c)<0)&&(\result==1)));
*/


int check(int a, int b, int c)
{
  if(((b*b)-(4*a*c))>=0)
  return 0;
  else
  return 1;
}

int main()
{
  int ret = check(2,1,1);
  return 0;
}
