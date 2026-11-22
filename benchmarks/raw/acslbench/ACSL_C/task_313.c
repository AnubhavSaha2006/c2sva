/*
 * Task 313 Natural Language:
 * Write an ACSL-annotated C function that swaps the values of two integers pointed to by
 * parameters a and b. The function requires valid pointers a and b, ensures that after execution,
 * *a equals the original *b and *b equals the original *a, and assigns only the memory locations
 * *a and *b.
 */

/*@ 
    requires \valid(a) && \valid(b);
    ensures A: *a==\old(*b);
    ensures B: *b==\old(*a);
    assigns *a, *b;
    */
void swap(int *a, int *b) {
    int temp = *a;
    *a = *b;
    *b = temp;


}