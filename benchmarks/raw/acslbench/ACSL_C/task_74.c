/*
 * Task 74 Natural Language:
 * Write an ACSL-annotated C function that calculates the area of the largest triangle inscribed in
 * a semicircle with given radius. The function returns NULL for negative radius values, otherwise
 * allocates and returns an integer pointer to the computed area (radius²). Requires radius to be
 * within INT_MIN/MAX bounds.
 */

#include <stdio.h>
#include <stdlib.h>
#include <limits.h>

/*@
  requires radius >= INT_MIN && radius <= INT_MAX;
  assigns \nothing;
  ensures (radius < 0) ==> (\result == NULL);
  ensures (radius >= 0) ==> (\result != NULL && *\result == radius * radius);
*/
int* AreaOfLargestTriangleInSemicircle(int radius) {
    if (radius < 0) {
        return NULL;
    }
    
    int* result = (int*)malloc(sizeof(int));
    if (result == NULL) {
        return NULL;
    }
    
    *result = radius * radius;
    return result;
}