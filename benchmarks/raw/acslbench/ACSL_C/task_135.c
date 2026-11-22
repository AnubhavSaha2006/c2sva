/*
 * Task 135 Natural Language:
 * Write an ACSL-annotated C function that clamps a value `v` between `min` and `max` bounds. The
 * function returns `v` if it is within the range [`min`, `max`], returns `min` if `v` is less than
 * `min`, and returns `max` if `v` is greater than `max`.
 */

/*@ assigns \nothing;
    ensures \result == \min(\max(min, v), max);
 */
int clamp(int v, int min, int max)
{
	int low = v > min ? v : min;
	return low < max ? low : max;
}
