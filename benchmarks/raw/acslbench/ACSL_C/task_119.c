/*
 * Task 119 Natural Language:
 * Write an ACSL-annotated C function that swaps elements at positions n1 and n2 in an integer
 * array arr of size n. The function requires valid indices (0 ≤ n1, n2 < n) and ensures the
 * elements at both positions are exchanged.
 */



/*@
    requires n >= 0;
    requires 0 <= n1 < n && 0 <= n2 < n;
    requires \valid_read(arr+(0..n-1));
    ensures (arr[n2] == \old(arr[n1])) && (arr[n2] == \old(arr[n1]));
*/
void array_swap(int* arr, int n, int n1, int n2) {
    int temp = arr[n1];
    arr[n1] = arr[n2];
    arr[n2] = temp;
}

// from this directory run: frama-c-gui -wp array_swap.c