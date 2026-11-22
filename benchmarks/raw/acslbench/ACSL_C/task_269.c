/*
 * Task 269 Natural Language:
 * Write an ACSL-annotated C program that defines a structure `A` with `size` and `pos` fields. The
 * program includes two functions: `foo` and `bar`. The `foo` function advances the `pos` field of
 * structure `A` by a given `length`, requiring that the new position does not exceed the
 * structure's `size` and respects unsigned integer limits. The `bar` function calls `foo` twice
 * with the same `length` but resets the position in between, effectively ensuring the final
 * position is advanced by `length` from the original. Both functions require valid pointers and
 * that `length` is at most 64.
 */


#include <limits.h>

struct A
{
    unsigned int size;
    unsigned int pos;
};

typedef struct A A;


/*@
    requires \valid(a);
    requires length <= 64;
    requires a->pos + length <= UINT_MAX;
    requires a->pos + length <= a->size;

    assigns  a->pos;

    ensures  a->pos == \old(a->pos) + length;
*/
void foo(A* a, unsigned int length)
{
    a->pos += length;
}

/*@
    requires \valid(a);
    requires length <= 64;
    requires a->pos + length <= UINT_MAX;
    requires a->pos + length <= a->size;

    assigns  a->pos;

    ensures  a->pos == \old(a->pos) + length;
*/
void bar(A* a, unsigned int length)
{
    foo(a, length);
    a->pos -= length;
    foo(a, length);
}


