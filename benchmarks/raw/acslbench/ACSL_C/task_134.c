/*
 * Task 134 Natural Language:
 * Write an ACSL-annotated C function that swaps the values of two integers through pointers. The
 * function requires valid pointers a and b, and ensures that after execution, *a contains the
 * original value of *b and vice versa.
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