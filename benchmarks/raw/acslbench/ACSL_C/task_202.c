/*
 * Task 202 Natural Language:
 * Write an ACSL-annotated C function that finds the index of the maximum element in an integer
 * array `a` of size `n` and swaps it with the last element. The function requires that the array
 * size `n` is greater than 0 and that the array is valid for indices 0 to n-1. It ensures that
 * after execution, the last element of the array is the maximum value found in the entire array.
 */

/*@

    requires n>0;



    requires \valid(a + (0..n-1));

    ensures \forall integer i;

    0<= i<n ==>a[n-1]>= a[i];

*/

void selectioninnerloop(int a[], int n)

{

    int max_idx = 0;



    /*@

    loop invariant \forall integer i;0<=i<j ==>a[max_idx]>= a[i];

    loop invariant 0<=j<=n;

    loop assigns max_idx,j;

    loop variant n-j;

*/

    for (int j = 0; j < n; j++)

    {

        if (a[j] > a[max_idx])

        {

            max_idx = j;

        }

    }

    int temp = a[n - 1];

    a[n - 1] = a[max_idx];

    a[max_idx] = temp;

}

