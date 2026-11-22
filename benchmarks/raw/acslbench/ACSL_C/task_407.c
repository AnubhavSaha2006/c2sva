/*
 * Task 407 Natural Language:
 * Write an ACSL-annotated C program that processes an integer array `t` of size 100. The function
 * `job` takes the array `t` and an integer `A` (where 50 ≤ A ≤ 100). It first initializes the
 * first 50 elements of `t` using function `f`, ensuring each element satisfies predicate `P`. Then
 * it updates elements from index `A` to 99 using function `g`. The main function calls `job` with
 * `A=50`.
 */


/*@ axiomatic A {
  @ predicate P(int x);
  @ }*/

/*@ ensures P(\result);
  @ assigns \nothing; */
int f(int i);

/*@ assigns \nothing; */
int g(int j);

void job(int *t, int A) {

  /*@ assert 50 <= A <= 100; */

  /*@ loop invariant 0 <= i <= 50;
    @ loop invariant \forall integer k; 0 <= k < i ==> P(t[k]);
    @ loop assigns i,t[0..49];
    @ */
  for(int i = 0; i < 50; i++) t[i] = f(i);

  /*@ loop invariant A <= j <= 100;
    @ loop assigns j,t[A..99];
    @ */
  for(int j = A; j < 100; j++) t[j] = g(j);

  /*@ assert \forall integer k; 0 <= k < 50 ==> P(t[k]); */

}

int T[100];

void main(void) {
  job(T, 50);
  //  job(T, 48);
}
