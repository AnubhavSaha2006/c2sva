/*
 * Task 261 Natural Language:
 * Write an ACSL-annotated C function that returns the next integer after a given positive integer
 * `a`. The function requires that `a` is greater than 0 and less than 2,147,483,647, and ensures
 * the result is greater than 1.
 */

/*@requires a>0 && a<2147483647;
ensures\result>1;*/
int next(int a)
{return a + 1;}