/*
 * Task 572 Natural Language:
 * Write an ACSL-annotated C function that increments each element of a 100-element integer array
 * `t` from 1 to 2. The function requires that the array is valid and initially contains all 1s,
 * and ensures that after execution, all elements become 2. The function modifies the entire array
 * but no other variables.
 */

int t[100];

/*@ requires \valid(t+(0..99)) && (\forall integer k ; 0 <= k < 100 ==> t[k] == 1) ;
    assigns t[0..99];
    ensures \valid(t+(0..99)) && (\forall integer k ; 0 <= k < 100 ==> t[k] == 2) ;
 */
void f(void)
{
   int i;

   /*@
        loop invariant 0 <= i <= 100 && (\forall integer k ; 0 <= k < i ==> t[k] == 2)
        && (\forall integer k ; i <= k < 100 ==> t[k] == 1) ;
        loop assigns t[0..99], i;
        loop variant 100 - i;
    */
   for (i=0; i<100; i++) t[i]++;
}
