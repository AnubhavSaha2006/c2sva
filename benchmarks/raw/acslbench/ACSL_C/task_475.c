/*
 * Task 475 Natural Language:
 * Write an ACSL-annotated C function that computes the absolute value of an integer a. The
 * function handles three cases: when a is zero (returns 0), when a is positive (returns a), and
 * when a is negative (returns -a). The behaviors are complete and disjoint, covering all possible
 * integer values within the range [-2147483647, 2147483647].
 */


#pragma JessieIntegerModel(exact)

/*@ requires -2147483647 <= a <= 2147483647;
    assigns \nothing;

    behavior zero:
        assumes a == 0;
        ensures \result == 0;

    behavior positive:
        assumes a > 0;
        ensures \result == a;

    behavior negative:
    	assumes a < 0;
    	ensures \result == -a;

    complete behaviors zero, positive, negative;
    disjoint behaviors zero, positive, negative;


*/
int abs(int a)
{
    if (a == 0)
        return 0;
    else if (a > 0)
        return a;
    else
        return -a;
}