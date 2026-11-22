/*
 * Task 118 Natural Language:
 * Write an ACSL-annotated C function that finds the maximum value in an integer array. The
 * function takes parameters `arr` (the array) and `n` (its length), requiring a non-empty valid
 * array. Returns the maximum value found in `arr`.
 */



/*@
    requires n > 0;
    requires \valid_read(arr+(0..n-1));
    ensures \result == \max(0, n-1, \lambda integer i; arr[i]);
    assigns \nothing;
*/
int array_max_advanced(int* arr, int n) {
    int max = arr[0];
    for (int i = 0; i < n; i++) {
        if (arr[i] > max) {
            max = arr[i];
        }
    }
    return max;
}