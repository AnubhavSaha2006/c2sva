/*
 * Task 375 Natural Language:
 * Write an ACSL-annotated C function that finds the maximum value in a double array `t` of size
 * `n`. The function requires `n > 0` and ensures the returned value `m` is the maximum element in
 * the array. The array index `i` iterates through the elements, updating `m` whenever a larger
 * value is found.
 */

/*@ requires n > 0;
  @ ensures \result == \max(0,n-1,\lambda integer k; t[k]);
  @*/
double max_array(double t[], int n) {
  double m; int i=0;
  goto L;
  do {
    if (t[i] > m) { L: m = t[i]; }
    /*@ invariant 
      @   0 <= i < n && m == \max(0,i,\lambda integer k; t[k]);
      @*/
    i++;
  }
  while (i < n); 
  return m;
}
