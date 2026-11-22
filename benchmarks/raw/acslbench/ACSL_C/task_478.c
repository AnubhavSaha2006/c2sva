/*
 * Task 478 Natural Language:
 * Write two ACSL-annotated C functions. The first function `f` takes an integer `x` and returns `x
 * + 1`. The second function `g` takes two integers `x` and `y` and returns `x + y + 1`. Both
 * functions require their input parameters to be within the range -1,000,000 to 1,000,000
 * (exclusive).
 */

/*@ requires -1000000 < x < 1000000;
 ensures \result == x + 1; 
 assigns \nothing; */
int f(int x) { return x+1; }

/*@ requires -1000000 < x < 1000000;
    requires -1000000 < y < 1000000;
 ensures \result == x + y + 1;
 assigns \nothing; */
int g(int x, int y) { return (x+y+1); }