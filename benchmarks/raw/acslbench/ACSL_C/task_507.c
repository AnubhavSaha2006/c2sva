/*
 * Task 507 Natural Language:
 * Write an ACSL-annotated C function that swaps two elements at indices i and j in an integer
 * array a of size n. The function requires valid array access for indices 0 to n-1, and ensures
 * that the elements at positions i and j are exchanged while all other elements remain unchanged.
 * The indices i and j must be within the valid range [0, n-1].
 */

/*@ predicate dom{L}(int k,int n) = k >= 0 && k <= n - 1;
 */
/*@
    requires A: n >= 0 && \valid (a +(0.. n-1));
    requires B: dom( i, n ) && dom( j, n );  
    assigns     a[i], a[j];
    ensures     \old(a[i]) == a[j] && \old(a[j]) == a[i];
    ensures     \forall int i1; dom(i1,n) ==>  
                \exists int i2; dom(i2,n) && (a[i1] == \old(a[i2]));
 */ 
void swap3(int *a, int n, int i, int j) {
  int temp = a[i];
  a[i] = a[j];
  a[j] = temp;
}