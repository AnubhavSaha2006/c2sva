/*
 * Task 293 Natural Language:
 * Write an ACSL-annotated C function that finds the first occurrence of element e in array t of
 * size n. The function returns the index of e if found (0 ≤ result < n), or -1 if e is not present
 * in the array. Requires array size n > 0 and a valid array t.
 */


/*@ requires n>0;
    requires \valid(t+(0..n-1));

    behavior success:
    assumes \exists integer i; 0<=i<n && t[i] == e;
    ensures 0 <= \result < n && t[\result]==e;

    behavior failure:
    assumes \forall integer i; 0<=i<n ==> t[i]!=e;
    ensures \result==-1;

*/
int index(int t[],int n,int e) {
/*@ loop invariant 0<=i<=n;
    loop invariant \forall integer j; 0<=j<i ==> t[j] == e;
    loop variant n-i;
*/
      for(int i=0;i<n;i++) {
            if( t[i] == e ) return i;
      }
      return -1;
}