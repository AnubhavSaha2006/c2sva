/*
 * Task 371 Natural Language:
 * Write an ACSL-annotated C function that computes the "left smaller neighbor" for each element in
 * an integer array. Given an input array `s` of size `length`, the function uses a stack-based
 * algorithm to fill the output array `left`, where each `left[i]` stores the index of the nearest
 * element to the left of `s[i]` that is smaller than `s[i]`. If no such element exists, `left[i]`
 * is set to 0. The function requires valid, separated memory blocks for `s`, `stack`, and `left`,
 * each of size `length`. It ensures that for every index `i`, `left[i]` is between 0 and `i`
 * inclusive, and that if `left[i]` is positive, then `s[left[i]-1]` is indeed smaller than `s[i]`.
 * Furthermore, it guarantees that for every `j` in the range `left[i]` to `i-1`, the element
 * `s[j]` is greater than or equal to `s[i]`.
 */

#include <stddef.h>

/*@ requires \valid(s + (0 .. length - 1));
    requires \valid(stack + (0 .. length - 1));
    requires \valid(left + (0 .. length - 1));
    requires \separated(stack + (0 .. length - 1),left + (0 .. length - 1));
    requires \separated(stack + (0 .. length - 1), s + (0 .. length - 1));
    requires \separated(left + (0 .. length - 1), s + (0 .. length - 1));

    assigns stack[0 .. length - 1], left[0 .. length - 1];

    ensures wf_left: \forall integer i; 0 <= i < length ==> 0 <= left[i] <= i;
    ensures left_small:
    \forall integer i; 0 <= i < length ==> 0 < left[i] ==>
     left[i] > 0 ==> s[left[i]-1] < s[i];
     ensures left_smallest:
     \forall integer i; 0 <= i < length ==>
        \forall integer j; left[i] <= j < i ==> s[j] >= s[i];
*/
void neighbor(int* s, size_t length, size_t* stack, size_t* left) {
  size_t sidx = 0;
  /*@ loop invariant s_untouched:
        \forall integer idx; 0 <= idx < length ==> s[idx] == \at(s[idx],Pre);
      loop invariant 0 <= x <= length;
      loop invariant 0 <= sidx <= x;
      loop invariant stack_left:
        \forall integer i; 0 <= i < sidx ==> 0 < stack[i] <= x;
      loop invariant wf_left:
        \forall integer i; 0 <= i < x ==> 0 <= left[i] <= i;
      loop invariant left_small:
        \forall integer i; 0 <= i < x ==> left[i] > 0 ==> s[left[i] - 1] < s[i];
      loop invariant left_smallest:
        \forall integer i; 0 <= i < x ==>
          \forall integer j; left[i] <= j < i ==> s[j] >= s[i];
      loop invariant stack_order:
        \forall integer i, j; 0<= i < j < sidx ==> 0 <= stack[i] < stack[j];
      loop invariant stack_sorder:
        \forall integer i, j;
          0<= i < j < sidx ==> s[stack[i]-1] < s[stack[j]-1];
      loop invariant s_begin:
      sidx > 0 ==>
        \forall integer i; 0<=i<stack[0] ==> s[i] >= s[stack[0] - 1];
      loop invariant step_n:
        x > 0 ==> sidx > 0 && stack[sidx - 1] == x;
      loop invariant stack_summary:
        \forall integer i; 0<= i < sidx - 1 ==>
          \forall integer j; stack[i] <= j < stack[i+1]-1 ==>
                 s[j] >= s[stack[i+1]-1];
      loop invariant stack_push: sidx > 0 ==> stack[sidx-1] == x;
      loop assigns x, sidx, stack[0 .. length - 1], left[0 .. length - 1];
      loop variant length - x;
   */
  for (size_t x = 0; x < length; x++) {
    /*@
       loop invariant s_untouched_inner:
        \forall integer idx; 0 <= idx < length ==> s[idx] == \at(s[idx],Pre);
        loop invariant 0 <= sidx <= \at(sidx,LoopEntry);
        loop invariant left_bigger:
          sidx > 0 ==>
          \forall integer i;
             stack[sidx-1] <= i < x ==> s[i] >= s[x];
        loop invariant stack_empty:
          sidx == 0 ==>
          \forall integer i; 0 <= i < x ==> s[i] >= s[x];
        loop assigns sidx;
        loop variant sidx;
     */
    while (sidx > 0 && s[stack[sidx-1]-1] >= s[x]) sidx--;
    if (sidx == 0) {
      left[x] = 0;
    } else {
      /*@ assert head_ok:
            \forall integer i; stack[sidx-1]<=i<x ==> s[i] >= s[x];
      */
      left[x] = stack[sidx - 1];
    }
	//@ assert a1: left[x] > 0 ==> s[left[x] - 1] < s[x];
label:
    stack[sidx] = x + 1;
    /*@ assert s_untouched:
        \forall integer idx; 0 <= idx < length ==> s[idx] == \at(s[idx],Pre);
    */
    //@ assert same: left[x] == \at(left[x], label);
    //@ assert a2: left[x] > 0 ==> s[left[x] - 1] < s[x];
    sidx++;
  }
}
