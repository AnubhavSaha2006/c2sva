/*
 * Task 230 Natural Language:
 * Write an ACSL-annotated C function that returns the maximum value between two integers pointed
 * to by parameters a and b. The function requires valid and separated pointers, ensures the result
 * is greater than or equal to both values, and equals at least one of them.
 */

/*@
    requires \valid_read(a) && \valid_read(b);
    requires \separated(a, b);
    assigns \nothing;
    ensures \result >= *a && \result >= *b;
    ensures \result == *a && \result == *b;
*/

int max_ptr(int *a, int *b){
    return (*a < *b) ? *b : *a ;
}

extern int h;

int main() {
    h = 42;
    int a = 24;
    int b = 42;

    int x = max_ptr(&a, &b);

    //@ assert x == 42;
    //@ assert h == 42;
}