/*
 * Task 577 Natural Language:
 * Write an ACSL-annotated C function that conditionally resets the value pointed to by `a` to zero
 * if the value pointed to by `b` is non-zero. The function requires valid and separated pointers
 * `a` and `b`, with their dereferenced values within a specified range. The function ensures that
 * if `*b` is not zero, then `*a` becomes zero; otherwise, `*a` remains unchanged.
 */

/*@
	requires \valid(a) && \valid_read(b);
	requires \separated(a,b);
	requires -10000000<*a<10000000 && -10000000<*b<10000000;
	assigns *a;
	ensures *b != 0 ==> *a==0;
	ensures *b == 0 ==> *a==\old(*a);
*/
void reset_1st_if_2nd_is_true(int* a,int const* b){
	if(*b){
		*a =0;
		//@ assert *a==0;
	}
}
int main(){
	int a =5, x =0;
	/*@ assigns a, x; */
	reset_1st_if_2nd_is_true(&a, &x);
	//@ assert a == 5 ;
	//@ assert x == 0 ;
	int const b =1;
	/*@ assigns a, x; */
	reset_1st_if_2nd_is_true(&a, &b);
	//@ assert a == 0 ;
	//@ assert b == 1 ;
}