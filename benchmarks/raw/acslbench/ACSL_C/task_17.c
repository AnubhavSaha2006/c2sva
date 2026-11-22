/*
 * Task 17 Natural Language:
 * Write an ACSL-annotated C function `lower_bound` that finds the first position in a sorted array
 * `a` of size `n` where the value `val` could be inserted while maintaining order. The binary
 * search implementation requires the input array to be sorted in non-decreasing order.  

Write an
 * alternative ACSL-annotated C function `lower_bound_raw` that performs the same search using
 * linear scan instead of binary search, also requiring a sorted input array `a` of size `n` and
 * returning the insertion position for `val`.
 */


/*@ requires \valid(a + (0..n-1));
    requires \forall integer i, integer j; 0 <= i < j < n ==> a[i] <= a[j];
    assigns \nothing;
    ensures 0 <= \result <= n;
    ensures \forall integer k; 0 <= k < \result ==> a[k] < val;
    ensures \forall integer k; \result <= k < n ==> val <= a[k];
*/
unsigned lower_bound(const int *a, unsigned n, int val)
{
   unsigned left = 0;
   unsigned right = n;
   unsigned middle = 0;

   /*@ loop invariant 0 <= left <= right <= n;
       loop assigns middle, left, right;
       loop invariant \forall integer i; 0 <= i < left ==> a[i] < val;
       loop invariant \forall integer i; right <= i < n ==> val <= a[i];
       loop variant right - left;
   */
   while (left < right) {
      middle = left + (right - left) / 2;
      if (a[middle] < val) {
         //@ assert \forall integer i; 0 <= i < middle+1 ==> a[i] < val;
         left = middle + 1;
      } else right = middle;
   }

   return left;
}

/*@ requires \valid(a + (0..n-1));
    requires \forall integer i, integer j; 0 <= i < j < n ==> a[i] <= a[j];
    assigns \nothing;
    ensures 0 <= \result <= n;
    ensures \forall integer k; 0 <= k < \result ==> a[k] < val;
    ensures \forall integer k; \result <= k < n ==> val <= a[k];
*/
unsigned lower_bound_raw(const int *a, unsigned n, int val)
{
   unsigned i = 0;
   /*@ loop invariant 0 <= i <= n;
       loop invariant \forall integer j; 0 <= j < i ==> a[j] < val;
       loop invariant \forall integer j; i <= j < n ==> a[j] >= a[i];
       loop assigns i;
       loop variant n - i;
    */
   for(; i < n; ++i) {
      if (a[i] >= val) {
         break;
      }
   }
   return i;
}

#ifdef OUT_OF_TASK
#include <stdio.h>
#define ARRAY_SIZE(array) (sizeof(array)/sizeof((array)[0]))
int main(void)
{
   int a[] = {0};
   int b[] = {0,1};
   int c[] = {0,1,2,3,4,5};
   int res;

   res = lower_bound(a, ARRAY_SIZE(a), 0);
   printf("res: %d\n", res);
   res = lower_bound(b, ARRAY_SIZE(b), 1);
   printf("res: %d\n", res);
   res = lower_bound(c, ARRAY_SIZE(c), 10);
   printf("res: %d\n", res);
}
#endif

