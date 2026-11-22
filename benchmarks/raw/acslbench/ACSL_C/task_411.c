/*
 * Task 411 Natural Language:
 * Write an ACSL-annotated C function that finds the maximum element in an integer array. The
 * function takes an array `arr` and its size `n` as parameters, requires that `n > 0` and the
 * array is valid for reading, and ensures the returned value is greater than or equal to every
 * element in the array.
 */



//find the max element of array

/*@
    requires n>0;
    requires \valid_read(arr+(0..n-1));
ensures \forall integer res;
    0<=res<n ==> \result >= arr[res];

*/


int checkPrime(int arr[], int n)
{
    int max = arr[0];
    /*@
    loop invariant \forall integer k;
        0<=k<i ==> max>=arr[k];
    loop invariant 1<=i<=n;
    loop assigns i, max;
    loop variant n-i;

    */

    
    for (int i = 1; i < n; i++)
    {
        if (max < arr[i])
        {
            max = arr[i];
        }
    }
    return max;

}