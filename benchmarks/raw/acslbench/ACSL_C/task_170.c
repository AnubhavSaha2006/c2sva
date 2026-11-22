/*
 * Task 170 Natural Language:
 * Write an ACSL-annotated C function that copies the value from input pointer i to output pointer
 * o. The function requires both pointers to be valid, ensures the value at o equals the value at i
 * after execution, and only modifies the value pointed by o.
 */



/*@ requires \valid(i) && \valid(o);

  @ assigns *o;



  @ ensures *o == *i;

*/

void OutAndIn(int *i, int *o) {

  *o = *i;

}



void OutAndIn_test()

{

	int a = 5;

	int b = 0;

	OutAndIn(&a, &b);

	//@ assert a == b;

}

