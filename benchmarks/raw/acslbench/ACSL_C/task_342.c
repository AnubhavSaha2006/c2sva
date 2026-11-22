/*
 * Task 342 Natural Language:
 * Write an ACSL-annotated C function that copies the value from one pointer's target to another
 * pointer's target. The function takes two double pointers `ppi` (input source) and `ppo` (output
 * destination), requires that all pointer dereferences are valid, and ensures the destination
 * contains the same value as the source after execution.
 */



#pragma JessieIntegerModel(exact)





/*@ requires \valid(ppi) && \valid(ppo) && \valid(*ppi) && \valid(*ppo);

  @ assigns **ppo;



  @ ensures **ppo == **ppi;

*/

void deref_OutAndIn(int **ppi, int **ppo) {

  **ppo = **ppi;

}



void deref_OutAndIn_test()

{

	int a = 5;

	int b = 0;

	int *pb = &b;

	int *pa = &a;

	deref_OutAndIn(&pa, &pb);

	//@ assert *pa == a;

	//@ assert *pb == a;

}

