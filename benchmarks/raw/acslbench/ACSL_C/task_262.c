/*
 * Task 262 Natural Language:
 * Write an ACSL-annotated C function that swaps the values of two integers. The function takes two
 * valid integer pointers `a` and `b` and assigns the old value of `*b` to `*a` and the old value
 * of `*a` to `*b`. The `swapped` predicate defines the post-condition where the values at the two
 * pointers are exchanged.
 */

/*@

    predicate swapped {L1, L2}(int* a, int* b) =

        \at(*a, L1) == \at(*b, L2) &&

        \at(*b, L1) == \at(*a, L2);

*/

/*@

    requires \valid(a);

    requires \valid(b);

    assigns *a;

    assigns *b;



    ensures *a == \old(*b);

    ensures *b == \old(*a);

    ensures swapped{Here, Old}(a,b);

*/

void swap (int* a, int* b )

{

    int c = *a;

    *a = *b;

    *b = c;

}

