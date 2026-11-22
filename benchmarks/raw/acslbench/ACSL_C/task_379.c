/*
 * Task 379 Natural Language:
 * Write an ACSL-annotated C function that swaps the values of two integers. The function requires
 * valid pointers a and b, assigns to the memory locations pointed to by a and b, and ensures that
 * after the call, the value at a equals the original value at b and the value at b equals the
 * original value at a.
 */

/*@
requires \valid(a) && \valid(b);
assigns *a, *b;
ensures *a == \old(*b) && *b == \old(*a);
*/
void swap(int* a, int* b) {
    int tmp = *a;
    *a = *b;
    *b = tmp;
}

int main(){
    int a = 37;
    int b = 91;
    swap(&a, &b);
}