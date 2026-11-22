/*
 * Task 351 Natural Language:
 * Write an ACSL-annotated C function that finds the first duplicate value in an integer array. The
 * function takes an array `a` of size `N` where each element is between 1 and MAXV (1,000,000). It
 * returns the first duplicate value found, or 0 if no duplicates exist. The function ensures the
 * result correctly identifies whether a duplicate exists and which value is duplicated.
 */


#include <stdbool.h>

#define MAXV 1000000

/*@ predicate noDouble{L}(int n, int *a) =
  @     \forall integer i; 0 <= i < n-1 ==>
  @         \forall integer j; i+1 <= j < n ==> \at(a[i], L) != \at(a[j], L);
  @*/


/*@ predicate existsDoubleRes{L}(int n, int *a, int res) =
  @     \exists integer i, j; 0 <= i < j < n ==> res == \at(a[i], L) && res == \at(a[j], L);
  @*/

/*@ predicate validateResult{L}(int n, int *a, int res) =
  @     res == 0 ?
  @         noDouble{L}(n, a) :
  @         existsDoubleRes{L}(n, a, res);
  @*/



/*@ requires 0 < N <= MAXV;
  @ requires \valid(a + (0..(N-1)));
  @ requires \forall integer i; 0 <= i < N ==> 1 <= a[i] <= MAXV;
  @ assigns \nothing;
  @ ensures 0 <= \result <= MAXV;
  @ ensures validateResult(N, a, \result);
  @*/
int findDouble(int N, int a[]) {
    bool f[MAXV];

    /*@ loop assigns i, f[0..(MAXV-1)];
      @ loop invariant 1 <= i <= (MAXV+1);
      @ loop invariant \forall integer k; 1 <= k < i ==> f[k-1] == false;
      @ loop variant MAXV-i;
      @*/
    for (int i = 1; i <= MAXV; ++i)
        f[i-1] = false;

    /*@ loop assigns i, f[0..(MAXV-1)];
      @ loop invariant 0 <= i <= N;
      @ loop invariant \forall integer k; 0<=k<i ==> f[a[k]-1] == true;
      @ loop invariant \forall integer k; 0 < k < i ==>
      @                    \forall integer l; 0 <= l < k ==>  a[k] != a[l];
      @ loop variant N-i;
      @*/
    for (int i = 0; i < N; ++i)
        if (f[a[i]-1])
            return a[i];
        else
            f[a[i]-1] = true;
    return 0;
}