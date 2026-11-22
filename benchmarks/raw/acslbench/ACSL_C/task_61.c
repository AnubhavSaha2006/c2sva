/*
 * Task 61 Natural Language:
 * Write an ACSL-annotated C function that calculates the surface area of a cylinder given its
 * radius and height. The function requires positive values for both radius and height, and returns
 * the area as a long integer. The result is computed as 2πr(r + h).
 */

#include <math.h>
#include <assert.h>

/*@
  requires radius > 0.0;
  requires height > 0.0;
  assigns \nothing;
  ensures \result == (long)(2.0 * M_PI * radius * (radius + height));
*/
long CylinderSurfaceArea(double radius, double height) {
    long area = (long)(2.0 * M_PI * radius * (radius + height));
    return area;
}