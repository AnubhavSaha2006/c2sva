/*
 * Task 427 Natural Language:
 * Write an ACSL-annotated C function that adds two arrays element-wise and stores the result in a
 * third array. The function takes three arrays a, b, c and an integer n as input, where n is the
 * size of the arrays. It requires that n is positive, all arrays are valid and separated in
 * memory, and elements of a and b are within the range [-10000, 10000]. The function ensures that
 * each element c[i] equals the sum of a[i] and b[i] for all valid indices.
 */

/*@
  @ requires n > 0;
  @ requires \valid_read(a+(0..n - 1));
  @ requires \valid_read(b+(0..n - 1));
  @ requires \valid(c+(0..n - 1));
  @ requires \separated(a+(0..n-1), b+(0..n-1), c+(0..n-1));
  @ requires \forall integer i; 0 <= i < n ==> -10000 <= a[i] <= 10000;
  @ requires \forall integer i; 0 <= i < n ==> -10000 <= b[i] <= 10000;
  @ ensures \forall integer i; 0 <= i < n ==> c[i] == a[i] + b[i];
  @ assigns c[0..n-1];
*/
void sum_array(int a[], int b[], int c[], int n) {
    /*@
      @ loop invariant 0 <= i <= n;
      @ loop invariant \valid_read(a+(0..n - 1));
      @ loop invariant \valid_read(b+(0..n - 1));
      @ loop invariant \valid(c+(0..n - 1));
      @ loop invariant \forall integer j; 0 <= j < i ==> c[j] == a[j] + b[j];
      @ loop assigns i, c[0..n-1];
      @ loop variant n - i;
    */
    for (int i = 0; i < n; ++i) {
        c[i] = a[i] + b[i]; 
    }
}