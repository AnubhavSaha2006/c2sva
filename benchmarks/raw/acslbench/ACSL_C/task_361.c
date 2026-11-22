/*
 * Task 361 Natural Language:
 * Write an ACSL-annotated C function that takes a pointer to a structure containing two integers
 * (a and b). The function checks if the pointer is valid and if the 'a' field is zero, then sets
 * the 'b' field to 0; otherwise sets 'b' to 1. The function only modifies the 'b' field of the
 * structure.
 */

// __in struct pointer

typedef struct s

{

  int a;

  int b;

} S;



/*@ requires \valid(ps);

  @ assigns ps->b;

*/

void f6(S *ps)

{

  if (ps)

  {

    if (ps->a == 0)

    {

      ps->b = 0;

    }

    else

    {

      ps->b = 1;

    }

  }

}



