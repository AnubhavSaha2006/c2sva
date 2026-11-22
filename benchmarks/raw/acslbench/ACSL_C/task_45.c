/*
 * Task 45 Natural Language:
 * Write an ACSL-annotated C function that calculates the volume of a cylinder given its radius and
 * height. The function requires positive radius and height values, and ensures the result is
 * within EPSILON tolerance of the exact mathematical volume (πr²h).
 */

#include <math.h>
#include <assert.h>

#define EPSILON 1e-2

/*@
  requires radius > 0.0;
  requires height > 0.0;
  assigns \nothing;
  ensures \abs(\result - (M_PI * radius * radius * height)) < EPSILON;
*/
double CylinderVolume(double radius, double height) {
    double volume = M_PI * radius * radius * height;
    return volume;
}