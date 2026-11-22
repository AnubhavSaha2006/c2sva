/*
 * Task 309 Natural Language:
 * Write an ACSL-annotated C function that takes a positive integer `y` greater than 10 and returns
 * a non-negative integer. The function's logic depends on the value of `y`, but due to the
 * precondition, it always executes the branch where `y` is positive, performing a series of
 * arithmetic operations on a local variable `x` before returning it.
 */

/*@ requires y > 10;
  @ ensures \result >= 0;
*/

int g(int y){
	int x=0;
	
	if(y>0){
		x=100;
		x=x+50;
		x=x-100;
	}else{
		x = x - 150;
		x=x-100;
		x=x+100;
	}
	return x;
}

int main(){
	int a = g(11);
	return a;
}