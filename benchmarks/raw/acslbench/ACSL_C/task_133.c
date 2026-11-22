/*
 * Task 133 Natural Language:
 * Write an ACSL-annotated C function that resets the value of *a to 0 if *b is true (non-zero).
 * The function requires valid and separated pointers a and b, preserves *b's value, and leaves *a
 * unchanged when *b is false.
 */

/*@
    requires \valid(a) && \valid_read(b);
    requires \separated(a, b);
    assigns *a;
    ensures \old(*b) ==> *a == 0;
    ensures ! \old(*b) ==> *a == \old(*a);
    ensures *b == \old(*b);
*/
void reset_1st_if_2nd_is_true(int* a, int const* b){
    if(*b) *a = 0 ;
}

int main(){
    int a = 5 ;
    int x = 0 ;
    reset_1st_if_2nd_is_true(&a, &x);
    //@ assert a == 5 ;
    //@ assert x == 0 ;

    int const b = 1 ;
    reset_1st_if_2nd_is_true(&a, &b);
    //@ assert a == 0 ;
    //@ assert b == 1 ;
}

