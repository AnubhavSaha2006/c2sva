/*
 * Task 143 Natural Language:
 * Write an ACSL-annotated C function that finds the index of the maximum element in an integer
 * array `u` of given `size`. The function requires a valid array of size greater than 0 and
 * ensures the returned index is valid and points to an element greater than or equal to all other
 * elements in the array. The function uses a global variable `max` to store the result.
 */

#define LENGTH 100

int vec[LENGTH];
int max;

/*@ requires \valid(u+(0..size-1)) && 0 < size;
  @ ensures 0 <= \result < size &&
  @     (\forall integer a; 0 <= a < size ==> u[a] <= u[\result]);
  @ assigns max; 
  @*/
int maxarray(int u[], int size) {
    int i = 1;
    max = 0;

    /*@ loop invariant
      @     0 <= max <  i <= size &&
      @     (\forall integer a;
      @         0 <= a < i ==> u[a] <= u[max]);
      @ loop assigns i, max;
      @ loop variant size-i;
      @*/
    while (i < size) {
        if (u[i] > u[max]) { max = i; }
        i++;
    }
    return max;
}

void main() {
    max = maxarray(vec, LENGTH);

    /*@ assert 0 <= max < LENGTH &&
      @     (\forall integer a; 
      @         0 <= a < LENGTH ==> vec[a] <= vec[max]);
      @*/
}

