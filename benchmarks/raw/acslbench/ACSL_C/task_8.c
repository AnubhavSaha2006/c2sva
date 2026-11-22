/*
 * Task 8 Natural Language:
 * Write an ACSL-annotated C function that computes the average of two integers (a and b) while
 * avoiding overflow. The function handles positive, negative, and mixed-sign inputs differently to
 * ensure correct results. The postcondition guarantees the returned value equals (a+b)/2.
 */

/*@ assigns \nothing;
    ensures \result == (a+b)/2;
*/
int average(int a, int b)
{
   int average = 0;

	int greater;
	int smaller;
	if (a > b) {
	   greater = a;
	   smaller = b;
	} else {
	   greater = b;
	   smaller = a;
	}
	if (a >= 0 && b >= 0) {
	   average = smaller + (greater - smaller) / 2;
	} else if (a < 0 && b < 0) {
	   average = greater + (smaller - greater) / 2;
	} else if ((a >= 0 && b <= 0) || (a <= 0 && b >= 0)) {
	   average = (a + b) / 2;
	}

   return average;
}

#ifdef OUT_OF_TASK
#include <stdio.h>

int main(void)
{
   printf("res: %d\n", average(3,30));
   printf("res: %d\n", average(-5,-20));
   printf("res: %d\n", average(7,-10));
   printf("res: %d\n", average(-2,15));
   return 0;
}
#endif
