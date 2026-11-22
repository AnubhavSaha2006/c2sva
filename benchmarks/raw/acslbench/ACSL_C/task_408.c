/*
 * Task 408 Natural Language:
 * Write an ACSL-annotated C function that swaps the integer values pointed to by two valid
 * pointers p and q. The function ensures that after execution, the value at p equals the original
 * value at q, and the value at q equals the original value at p.
 */

/*@
requires \valid(p);
requires \valid(q);
assigns *p;
assigns *q;
ensures *p == \old(*q);
ensures *q == \old(*p);
*/
void swap(int* p, int* q){
	int tmp = *p;
	*p=*q;
	*q=tmp;
	return;
}