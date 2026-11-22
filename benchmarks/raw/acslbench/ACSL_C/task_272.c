/*
 * Task 272 Natural Language:
 * Write an ACSL-annotated C function that computes the absolute value of an integer n. The
 * function requires n ≠ 0 and has two behaviors: when n is positive, it returns n unchanged; when
 * n is negative, it returns -n. The behaviors are complete and disjoint.
 */



/*@
    requires n>0;
    behavior one:
    assumes n>0;
    ensures \result == n;

    behavior two:

    assumes n<0;
    ensures \result == (-1)*n;

    complete behaviors;
    disjoint behaviors;

 */

int  ABS(int n)
{

    if (n < 0)
    {
        n = (-1) * n;
    }

    return n;
}