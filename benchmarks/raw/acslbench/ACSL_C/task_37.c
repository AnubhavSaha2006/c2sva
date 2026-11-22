/*
 * Task 37 Natural Language:
 * Write an ACSL-annotated C function that calculates the volume of a sphere given its radius. The
 * function requires a positive radius value and ensures the result is accurate within 1e-9 of the
 * exact mathematical calculation. The parameter is named 'radius'.
 */

#include <stdio.h>
#include <math.h>
#include <assert.h>

/*@
  requires radius > 0.0;
  assigns \nothing;
  ensures \abs(\result - (4.0/3.0 * M_PI * radius * radius * radius)) < 1e-9;
*/
double SphereVolume(double radius) {
    double volume = (4.0 / 3.0) * M_PI * radius * radius * radius;
    return volume;
}