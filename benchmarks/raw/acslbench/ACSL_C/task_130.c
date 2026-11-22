/*
 * Task 130 Natural Language:
 * Write an ACSL-annotated C function that increments the value at pointer `a` by the value at
 * read-only pointer `b`. The function requires valid and separated pointers, ensures `a` is
 * modified while `b` remains unchanged, and returns the new value of `a`.
 */

/*@
    requires \valid(a) && \valid_read(b);
    requires \separated(a, b);
    assigns *a;
    ensures *a == \old(*a) + *b;
    ensures *b == \old(*b);
*/
int incr_a_by_b(int* a, int const* b){
    *a += *b;
    return *a;
}