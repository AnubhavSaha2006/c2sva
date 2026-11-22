/*
 * Task 443 Natural Language:
 * Write an ACSL-annotated C function that checks if a given integer array `a` of size `n` is
 * sorted in non-decreasing order. The function returns 1 if the array is sorted (each element is
 * less than or equal to the next), and 0 otherwise. The function requires that the array size `n`
 * is greater than 0 and that the array is valid for indices 0 to n-1.
 */

/*@ requires n>0 && \valid(a+(0..n-1));
  @ assigns \nothing;
  @ behavior success:
  @   assumes \forall integer i; 0<=i<n-1 ==> a[i]<=a[i+1];
  @   ensures \result==1;
  @ behavior failure:
  @   assumes \exists integer i; 0<=i<n-1 && a[i]>a[i+1];
  @   ensures \result==0;
  @ complete behaviors success, failure;
  @ disjoint behaviors success, failure;
*/
int isIncreasing(int a[], int n){
  /*@ loop invariant 0<=i<=n;
      loop invariant \forall integer j; 0<=j<i ==> a[j]<=a[j+1];
      loop assigns i;
      loop variant n-i;
  */
  for(int i=0;i<n-1;i++){
    if(a[i]>a[i+1]) return 0;
  }
  return 1;
}