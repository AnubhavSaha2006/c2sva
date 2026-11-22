/*
 * Task 203 Natural Language:
 * Write an ACSL-annotated C function that takes an array `arr` of size `size` and an index `addr`.
 * The function requires that all array elements are valid indices within the array bounds (0 ≤
 * arr[i] < size) and that `addr` is a valid index. The function assigns zero to the array element
 * at the position specified by the value stored in `arr[addr]`.
 */

/*@ requires \valid(arr + (0..size-1));
    requires \forall integer i; 0 <= i < size ==> 0 <= arr[i] < size;
    requires 0 <= addr < size;
    assigns arr[0..size-1];
    ensures arr[\at(arr[addr],Pre)] == 0;
 */
void indaddr(unsigned size, int arr[size], unsigned addr)
{
	arr[arr[addr]] = 0;
}
