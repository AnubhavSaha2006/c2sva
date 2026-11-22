/*
 * Task 38 Natural Language:
 * Write an ACSL-annotated C function that calculates the surface area of a sphere given its
 * radius. The function requires a positive radius value and returns the result of 4πr². The
 * parameter is named 'radius'.
 */

#include <stdio.h>
#include <assert.h>
#include <math.h>

/*@
  requires radius > 0.0;
  assigns \nothing;
  ensures \result == 4.0 * M_PI * radius * radius;
*/
double SphereSurfaceArea(double radius) {
    const double pi = M_PI;
    double area = 4.0 * pi * radius * radius;
    return area;
}