/*
 * Task 98 Natural Language:
 * Write an ACSL-annotated C function that swaps the values of two integers through pointers. The
 * function requires valid pointers a and b, and ensures *a contains *b's original value and vice
 * versa. The main program demonstrates swapping values 42 and 37.
 */

 /*@
 requires \valid(a) && \valid(b);
 assigns *a, *b;
 ensures *a == \old(*b);
 ensures *b == \old(*a);
 */
 void swap(int* a, int* b){
 int tmp = *a;
 *a = *b;
 *b = tmp;
 }

 int main(){
 int a = 42;
 int b = 37;

 swap(&a, &b);

 //@ assert a == 37 && b == 42;

 return 0;
 }
