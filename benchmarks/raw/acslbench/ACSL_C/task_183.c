/*
 * Task 183 Natural Language:
 * Write an ACSL-annotated C function that returns the maximum value between two integers pointed
 * to by parameters a and b. The function requires valid pointers and that both integers are within
 * the range -10,000,000 to 10,000,000. The result ensures it equals one of the input values and is
 * greater than or equal to the other.
 */

/*@
	requires \valid(a) && \valid(b);
	requires -10000000<*a<10000000 && -10000000<*b<10000000;
	ensures \result==*a && \result>=*b || \result==*b && \result>=*a;
*/
int max_ptr(int* a,int* b){
	return(*a > *b) ? *a: *b ;
}
extern int h ;
int main(){
	h =42;
	int a =24,b =42;
	int x= max_ptr(&a,&b);
}
