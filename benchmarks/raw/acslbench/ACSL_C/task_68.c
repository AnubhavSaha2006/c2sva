/*
 * Task 68 Natural Language:
 * Write an ACSL-annotated C function that calculates the circumference of a circle given its
 * radius. The function requires a positive radius value and ensures the result is accurate within
 * 1e-12 of 2πr. The parameter is named 'radius'.
 */

#include <stdio.h>
#include <math.h>
#include <assert.h>

/*@
  requires radius > 0.0;
  assigns \nothing;
  ensures \abs(\result - (2.0 * M_PI * radius)) < 1e-12;
*/
double CircleCircumference(double radius) {
    const double PI = 3.14159265358979323846;
    double circumference = 2.0 * PI * radius;
    return circumference;
}