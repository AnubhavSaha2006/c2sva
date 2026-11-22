/*
 * Task 314 Natural Language:
 * Write an ACSL-annotated C program that uses a function `get_index` to obtain a valid array index
 * between 0 and N-1 (where N=12), then assigns the value 3 to the array element at that index. The
 * `get_index` function is specified to return an integer in the range [0, N-1].
 */


#define N 12

int array[ N ];

/*@ ensures 0 <= \result < N; */
int get_index(void);

void main()
{
  int i = get_index();
  array[ i ] = 3;
}

#ifndef FRAMA_C
int get_index(void)
{
  int i = 7;
  return i;
}
#endif
