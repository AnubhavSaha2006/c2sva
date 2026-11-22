/*
 * Task 84 Natural Language:
 * Write an ACSL-annotated C function that performs a left bit rotation on a 32-bit unsigned
 * integer n by d positions. The function requires the rotation amount d to be between 0 and 31 (0
 * ≤ d < 32) and returns the rotated value.
 */

#include <stdint.h>
#include <assert.h>

/*@
  requires 0 <= d < 32;
  assigns \nothing;
  ensures (n << d) | (n >> (32 - d)) ==> \result;
*/
uint32_t RotateLeftBits(uint32_t n, int d) {
    uint32_t result = (n << d) | (n >> (32 - d));
    return result;
}