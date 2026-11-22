/*
 * Task 417 Natural Language:
 * Write an ACSL-annotated C function that checks if a given square matrix is symmetric. The
 * function takes a 2D integer matrix and its size `len` as parameters, and returns `true` if the
 * matrix is symmetric (where each element matrix[i][j] equals matrix[j][i] for all i,j), and
 * `false` otherwise. The specification requires that the matrix is valid for indices 0 to len-1 in
 * both dimensions and that len is non-negative.
 */



#include <stdbool.h>


/*@ requires \valid(matrix[0..len-1]+(0..len-1));
  @ requires \valid(matrix+(0..len-1));
  @ requires len >= 0;
  @ assigns \nothing;
  @ behavior symm:
  @    assumes \forall integer i,j; 0 <= i < len ==> 0 <= j < len ==> matrix[i][j] == matrix[j][i];
  @    ensures \result == true;
  @ behavior nosymm:
  @    assumes \exists integer i,j; 0 <= i < len && 0 <= j < len && matrix[i][j] != matrix[j][i];
  @    ensures \result == false;
  @ complete behaviors;
  @ disjoint behaviors;
  @ */
_Bool is_symmetric(int **matrix, int len) {
  /*@loop invariant 0 <= i <= len;
    @loop invariant \forall integer k, l; 0 <= k < i ==> 0 <= l < k ==> matrix[k][l] == matrix[l][k];
    @loop assigns i;
    @loop variant len - i;
    @ */
  for (int i = 0; i < len; i++)
    /*@loop invariant 0 <= j <= i;
    @loop invariant \forall integer k; 0 <= k < j ==> matrix[i][k] == matrix[k][i];
    @loop assigns j;
    @loop variant i - j;
    @ */
    for (int j = 0; j < i; j++)
      if (matrix[i][j] != matrix[j][i])
        return false;
  return true;
}
