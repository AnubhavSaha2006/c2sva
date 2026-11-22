/*
 * Task 218 Natural Language:
 * Write an ACSL-annotated C function that swaps the values of two integers pointed to by
 * parameters a and b. The function requires that both pointers are valid and ensures that after
 * execution, the value at a equals the original value at b, and the value at b equals the original
 * value at a.
 */

/*@requires\valid(a) &&\valid(b);
ensures *a ==\old(*b) && *b ==\old(*a);*/
int swap(int *a, int *b)
{
    int temp = *a;
    *a = *b;
    *b = temp;
    return 0;
}