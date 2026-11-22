/*
 * Task 394 Natural Language:
 * Write four ACSL-annotated C functions:

1.  **max3**: Takes three integers a, b, c and returns
 * the largest value among them. The ensures clauses guarantee the result is greater than or equal
 * to each input and is equal to one of them.

2.  **double_**: Takes an integer array `t` of size
 * `n` (where 0 < n <= 100000000) and modifies it so that every element at index `i` becomes `2*i`.
 * The ensures clause guarantees the final array state.

3.  **increasing**: Checks if an integer
 * array `arr` of size `n` (n > 0) is strictly increasing. Returns 1 if every element is greater
 * than the previous one, 0 otherwise.

4.  **monotomic**: Checks the monotonicity of an integer
 * array `arr` of size `n` (n > 0). Returns 1 if the array is strictly increasing, -1 if strictly
 * decreasing, and 0 otherwise. It uses the previously defined `increasing` and `decreasing`
 * functions.
 */

// =================== Exercies 1 =================== 
/*@
  @ ensures \result >= a;
  @ ensures \result >= b;
  @ ensures \result >= c;
  @ ensures \result == a || \result == b || \result == c;
  @ assigns \nothing;
*/
int max3(int a, int b, int c) {
    if (a >= b && a >= c) return a;
    if (b >= b && b >= c) return b;
    return c;
}


// =================== Exercies 2 ===================
/*@
  @ requires 0 < n <= 100000000;
  @ requires \valid(t+(0..n - 1));
  @
  @ ensures \forall int i; 0 <= i < n ==> t[i] == 2 * i;
*/
void double_(int t[], int n) {
    /*@
      @ loop invariant 0 <= i <= n;
      @ loop invariant \forall int j; 0 <= j < i ==> t[j] == 2 * j;
      @ loop   variant n - i;
    */
    for (int i = 0; i < n; ++i) {
        t[i] = 2 * i;
    }
}


// =================== Exercies 3 ===================
/*@
  @ requires \valid(arr+(0.. n - 1));
  @ requires n > 0;
  @
  @ assigns \nothing;
  @ behavior true_:
  @   assumes \forall int i; 1 <= i < n ==> arr[i] > arr[i - 1];
  @   ensures \result == 1;
  @ behavior false_:
  @   assumes \exists int i; 1 <= i < n &&  arr[i] <=  arr[i - 1];
  @   ensures \result == 0; 
*/
int increasing(int arr[], int n) {
    /*@
      @ loop invariant 1 <= i <= n;
      @ loop invariant \forall int j; 1 <= j < i ==> arr[j] > arr[j - 1];
      @ loop   variant n - i;
    */
    for (int i = 1; i < n; ++i) {
        if (arr[i] <= arr[i - 1]) return 0;
    }
    return 1;
}



// =================== Exercies 4 ===================

/*@
  @ requires \valid(arr+(0.. n - 1));
  @ requires n > 0;
  @
  @ assigns \nothing;
  @ behavior true_:
  @   assumes \forall int i; 1 <= i < n ==> arr[i] < arr[i - 1];
  @   ensures \result == 1;
  @ behavior false_:
  @   assumes \exists int i; 1 <= i < n &&  arr[i] >= arr[i - 1];
  @   ensures \result == 0; 
*/
int decreasing(int arr[], int n) {
    /*@
      @ loop invariant 1 <= i <= n;
      @ loop invariant \forall int j; 1 <= j < i ==> arr[j] < arr[j - 1];
      @ loop   variant n - i;
    */
    for (int i = 1; i < n; ++i) {
        if (arr[i] >= arr[i - 1]) return 0;
    }
    return 1;
}


/*@
  @ requires \valid(arr+(0.. n - 1));
  @ requires n > 0;
  @
  @ assigns \nothing;
  @ behavior increaing:
  @   assumes \forall int i; 1 <= i < n ==> arr[i] > arr[i - 1];
  @   ensures \result == 1;
  @ behavior decreasing:
  @   assumes \forall int i; 1 <= i < n ==> arr[i] < arr[i - 1];
  @   ensures \result == -1;
  @ behavior none:
  @   assumes \exists int i; 1 <= i < n &&  arr[i] <= arr[i - 1];
  @   assumes \exists int i; 1 <= i < n &&  arr[i] >= arr[i - 1];
  @   ensures \result == 0;
*/
int monotomic(int arr[], int n) {
    if (increasing(arr, n)) return  1;
    if (decreasing(arr, n)) return -1;
    return 0;
}