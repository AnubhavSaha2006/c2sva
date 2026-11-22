/*
 * Task 344 Natural Language:
 * Write an ACSL-annotated C program that computes the sum of two preprocessor macros X and Y,
 * where X is defined as 1 + 2 + 3 and Y is defined as 5 + 6 + 7. The function ensures the return
 * value equals the sum of these macro expansions.
 */


#define X 1 /* multi-line

           comment */ + 2 + /*
             bla
           */ 3

#define Y 5 + \
          6 + \
          7

/*@ ensures \result == X+Y; */
int main(void)
{
    return X+Y;
}

