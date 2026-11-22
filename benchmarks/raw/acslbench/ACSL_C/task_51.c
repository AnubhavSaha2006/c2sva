/*
 * Task 51 Natural Language:
 * Write an ACSL-annotated C function that performs element-wise addition of two 2D integer arrays
 * (a and b) with dimensions rows×cols, returning a newly allocated result matrix. The function
 * requires valid input matrices and ensures each element result[i][j] equals a[i][j] + b[i][j].
 */

#include <stdlib.h>

/*@
  requires rows > 0 && cols > 0;
  requires \valid(a+(0..rows-1)) && \valid(b+(0..rows-1));
  requires \forall integer i; 0 <= i < rows ==> \valid(a[i]+(0..cols-1)) && \valid(b[i]+(0..cols-1));
  assigns \nothing;
  ensures \valid(\result+(0..rows-1));
  ensures \forall integer i; 0 <= i < rows ==> \valid(\result[i]+(0..cols-1));
  ensures \forall integer i,j; 0 <= i < rows && 0 <= j < cols ==> \result[i][j] == a[i][j] + b[i][j];
*/
int** IndexWiseAddition(int** a, int** b, size_t rows, size_t cols) {
    int** result = (int**)malloc(rows * sizeof(int*));
    
    /*@
      loop invariant 0 <= i <= rows;
      loop invariant \forall integer k; 0 <= k < i ==> \valid(result[k]+(0..cols-1));
      loop invariant \forall integer k,j; 0 <= k < i && 0 <= j < cols ==> result[k][j] == a[k][j] + b[k][j];
      loop assigns i, result, result[0..rows-1];
      loop variant rows - i;
    */
    for (size_t i = 0; i < rows; ++i) {
        result[i] = (int*)malloc(cols * sizeof(int));
        
        /*@
          loop invariant 0 <= j <= cols;
          loop invariant \forall integer k; 0 <= k < j ==> result[i][k] == a[i][k] + b[i][k];
          loop assigns j, result[i][0..cols-1];
          loop variant cols - j;
        */
        for (size_t j = 0; j < cols; ++j) {
            result[i][j] = a[i][j] + b[i][j];
        }
    }
    
    return result;
}