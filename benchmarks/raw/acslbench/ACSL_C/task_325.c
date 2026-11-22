/*
 * Task 325 Natural Language:
 * Write an ACSL-annotated C function that performs a linear search for a positive value `val` in
 * an integer array `arr` of size `n`. The function requires a valid, non-empty array (`n > 0`) and
 * a positive search value (`val > 0`). It returns 1 if the value is found in the array and 0 if it
 * is not found.
 */

//binary search

/*@
    requires n>0;
    requires \valid_read(arr+(0..n-1));
    requires val>0;

    behavior notfound:

    assumes \forall integer nf;
    0<=nf<n ==> arr[nf]!= val;
    ensures \result == 0;

    behavior found:
    assumes \exists integer f;
    0<=f<n && arr[f]==val;
    ensures \result == 1;

    complete behaviors;
    disjoint behaviors;


*/


int Linear(int arr[], int n, int val){

/*@
    loop invariant 0<=i<=n;
    loop invariant \forall integer k;
    0<=k<i ==> arr[k]!=val;
    loop assigns i;
    loop variant n-i;
*/
for(int i=0;i<n;i++){

    if(val==arr[i]){
        return 1;
    }
}
    return 0;
}