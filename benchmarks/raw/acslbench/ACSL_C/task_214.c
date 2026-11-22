/*
 * Task 214 Natural Language:
 * Write an ACSL-annotated C function that checks if every element in array `arr` of size `n`
 * equals its index. The function returns 1 if all elements satisfy `arr[i] == i`, and 0 otherwise.
 * Requires array size `n > 0` and valid memory for the array.
 */



/*@
    requires n>0;
    requires \valid_read(arr+(0..n-1));
    behavior one:
        assumes \forall integer j;
        0<=j<n ==> arr[j]==j;
        ensures \result == 1;
        
    behavior two:

    assumes \exists integer j;
    0<=j<n && arr[j]!=j;
    ensures \result == 0;

    complete behaviors;
    disjoint behaviors;

*/

int checkIndex(int arr[], int n){

    /*@
        loop invariant 0<=i<=n;
        loop invariant \forall integer k;
        0<=k<i ==> arr[k]==k;
        loop assigns i;
        loop variant n-i;
    
    */

    for (int i = 0; i < n; i++)
    {
        if(arr[i]!=i){
            return 0;
        }
    }

    return 1;
}