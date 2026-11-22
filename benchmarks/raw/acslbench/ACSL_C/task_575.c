/*
 * Task 575 Natural Language:
 * Write an ACSL-annotated C function that compares two integer arrays s and t of size n. The
 * function returns 1 if all elements in both arrays are equal, and 0 if any corresponding elements
 * differ. Requires array size n > 0 and valid memory allocation for both arrays.
 */

/*@ requires n>0 && \valid(s+(0..n-1)) && \valid(t+(0..n-1));
    assigns \nothing;
    behavior success:
      assumes \forall integer i; 0<=i<n ==> s[i]==t[i];
      ensures \result == 1;
    behavior failure:
      assumes \exists integer i; 0<=i<n && t[i]!=s[i];
      ensures \result == 0;
    complete behaviors success, failure;
    disjoint behaviors success, failure;
*/
int compare(int s[], int t[], int n){
  /*@ loop invariant 0<=i<=n;
      loop invariant \forall integer j; 0<=j<i ==> s[j]==t[j];
      loop invariant i == n ==> (\forall integer k; 0<=k<n ==> s[k] == t[k]);
      loop assigns i;
      loop variant n-i;
  */
  for(int i=0;i<n;i++){
    if(s[i]!=t[i]) return 0;
  }
  return 1;
}