/*
 * Task 248 Natural Language:
 * Write an ACSL-annotated C function that implements the partition algorithm for quicksort. The
 * function takes an array `A` and indices `p` and `r`, requiring that `0 <= p <= r` and the array
 * segment from `p` to `r` is valid. It partitions the array around the pivot `A[r]` and returns
 * the final index of the pivot. A helper `swap` function exchanges elements at indices `i` and `j`
 * within the valid range `start` to `end`.
 */

/* @ requires \valid(t+(start..end)) && start <= i <= end && start <= j <= end
 */
void swap(int t[], int i, int j, int start, int end) {
    int tmp = t[i];
    t[i] = t[j];
    t[j] = tmp;
}

/*@ requires 0 <= p <= r && \valid(A+(p..r)); @*/
int partition (int A[], int p, int r) 
{ 
    int x = A[r]; 
    int j, i = p-1; 

    /*@ loop invariant p <= j <= r && p-1 <= i < j;
      @ loop assigns i, j, A[p..r];
      @ loop variant r-j;
      @*/
    for (j=p; j<r; j++) 
        if (A[j] <= x) { 
            i++; 
            swap(A, i, j, p, r);
        } 
    swap(A,i+1,r,p,r);
    return i+1; 
}

