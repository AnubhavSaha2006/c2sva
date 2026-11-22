/*
 * Task 435 Natural Language:
 * Write two ACSL-annotated C functions. The first function `bidon` takes global variables A, B,
 * and C. If C is non-zero, it assigns C to both A and B; otherwise, A and B remain unchanged
 * (100). The second function `all_zeros` checks if all elements in array t of size n are zero,
 * returning 1 if true and 0 otherwise. Requires a valid array of size n ≥ 0.
 */

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-variable"
int A, B, C;
#pragma GCC diagnostic pop

/*@ requires A==100 && B==100;
    assigns A,B;
    ensures C==0 ==> A==100 && B==100;
    ensures C!=0 ==> A==C && B==C;*/
void bidon(){
  if ( C != 0 ){
    A = C;
    B = C;
  }
}

/*@ requires \valid ( t +(0.. n -1)) && n >= 0;
  @ assigns \nothing;
  @ ensures \result == 1 <==> ( \forall integer i ; 0 <= i < n ==> t [ i ] == 0);
  @ ensures \result == 0 <==> ( \exists integer i ; 0 <= i < n && t [ i ] != 0);
  @*/
int all_zeros(int t[], int n) {
  int k;
  /*@ loop invariant 0 <= k <= n;
      @ loop invariant \forall integer i ; 0 <= i < k ==> t [ i ] == 0;
      @ loop assigns k;
      @ loop variant n-k;
   */
  for(k = 0; k < n; k++) {
    //@ assert 0 <= k < n;
    if (t[k] != 0)
      return 0;
  }
    //@ assert k == n;
  return 1;
}
