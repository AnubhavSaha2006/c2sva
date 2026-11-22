/*
 * Task 286 Natural Language:
 * Write an ACSL-annotated C function that increments all elements of array `a` of size `n` by a
 * constant value `c`. The function requires that `n` is non-negative and the array is valid for
 * reading. The function ensures that after execution, every element `a[k]` (for `0 <= k < n`)
 * equals its original value plus `c`.
 */

#include <stdio.h>
#include <stdlib.h>

/*@
    requires n >= 0;
    requires \valid_read(a+(0..n-1));
    ensures \forall integer k; 0 <= k < n ==> a[k] == a[k]+c;
*/
void arrayinc(int *a,int n,int c){
    /*@
        loop invariant 0 <= p <= n;
        loop invariant \forall integer k; 0 <= k < p ==> a[k] == a[k]+c;
        loop invariant \forall integer k; p <= k < n ==> a[k] == a[k];
        loop assigns p,a[0..(n-1)];
     */   
    for(int p = 0;p < n;p++){
        a[p] = a[p] + c;
    }
}
int main(){
    int a[] = {1,2,3,4,5};
    int n = 5;
    int c = 2;
    arrayinc(a,n,c);
    return 0;
}
