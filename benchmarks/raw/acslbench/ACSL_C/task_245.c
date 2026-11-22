/*
 * Task 245 Natural Language:
 * Write an ACSL-annotated C function that returns the maximum of two integers a and b. The
 * function ensures the result equals either a (when a ≥ b) or b (when b ≥ a).
 */

/*@ensures\result == a &&\result>b
|| \result>a &&\result == b
|| \result == a &&\result == b;*/
int max(int a, int b)
{return a>b ?  a :  b;}