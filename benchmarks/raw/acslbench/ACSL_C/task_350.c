/*
 * Task 350 Natural Language:
 * Write an ACSL-annotated C function that compares two integer arrays `a` and `b` of size `n` for
 * equality. The function returns 1 if all elements in both arrays are equal, and 0 if any
 * corresponding elements differ. The function requires valid read access to both arrays and a non-
 * negative array size `n`.
 */

#include <stdio.h>
#include <stdlib.h>
/*@
requires \valid_read(a + (0..n-1));
requires \valid_read(b + (0..n-1));
requires n >= 0;
assigns \nothing;


behavior if_equal:
assumes \forall integer k; 0<=k<n ==> a[k] == b[k];
ensures \result == 1;

behavior if_not_equal:
assumes \exists integer k;0<=k<n && a[k] != b[k];
ensures \result == 0; 
*/
int equal(int *a, int n, int *b){
     int i;
     i = 0;
    /*@ 
        loop invariant 0 <= i <= n;
        loop invariant \forall integer k; 0 <= k < i ==> a[k] == b[k];
        loop assigns i;
        loop variant n-i;
    */ 
    for (i = 0; i < n; i++) {
    
        if (a[i] != b[i]){
            return 0;
        }
    }
    return 1;
}
int main(){
    int a[] = {1,2,3,4,5};
    int b[] = {1,2,3,4,5};
    equal(a,5,b);
    return 0;
}

