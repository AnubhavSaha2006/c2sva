/*
 * Task 402 Natural Language:
 * Write an ACSL-annotated C function that swaps the values of two integers pointed to by a and b.
 * The function requires valid pointers and ensures that after execution, the values at the two
 * memory locations are exchanged.
 */

/*@ predicate swapped{L1,L2}(int *a, int *b) = 
\at(*a, L2) == \at(*b,L1) && 
\at(*b, L2) == \at(*a,L1);
*/
/*@ requires \valid(a);
requires \valid(b);
assigns *a, *b;
ensures swapped{Pre, Post}(a, b);
*/
void swap(int *a, int *b)
{
	int tmp = *a;
	*a = *b;
	*b = tmp;
}
