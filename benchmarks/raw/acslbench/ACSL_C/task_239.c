/*
 * Task 239 Natural Language:
 * Write an ACSL-annotated C function that finds the maximum value in an integer array `arr` of
 * size `n`. The function requires that the array size `n` is greater than 0 and that the array is
 * valid for reading. The function ensures the result is the maximum value among all elements in
 * the array.
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