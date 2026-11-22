/*
 * Task 48 Natural Language:
 * Write an ACSL-annotated C function that calculates the volume of a cone given its radius and
 * height. The function requires both radius and height to be positive values and ensures the
 * result is accurate within a tolerance of 1e-12.
 */

#include <math.h>
#include <assert.h>

/*@
  requires radius > 0.0;
  requires height > 0.0;
  assigns \nothing;
  ensures \abs(\result - ((1.0/3.0) * M_PI * radius * radius * height)) < 1e-12;
*/
double ConeVolume(double radius, double height) {
    double volume = (1.0 / 3.0) * M_PI * radius * radius * height;
    return volume;
}