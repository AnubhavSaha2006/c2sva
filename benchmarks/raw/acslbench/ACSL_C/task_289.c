/*
 * Task 289 Natural Language:
 * Write an ACSL-annotated C function that returns the size of a char type and ensures the result
 * is less than or equal to the size of an int type.
 */

/*@ ensures \result <= sizeof(int); */
int f() { return sizeof(char); }
