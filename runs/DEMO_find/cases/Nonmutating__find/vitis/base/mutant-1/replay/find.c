#define main c2sva_original_main

#include "find.h"

size_type find(const value_type* a, size_type n, value_type v)
{
  
  for (size_type i = 1u; i < n; i++) {
    if (a[i] == v) {
      return i;
    }
  }

  return n;
}


#undef main
int printf(const char *, ...);   /* no <stdio.h>: it may clash with design names */
int main(void) {
  value_type a_m[4] = {-536870913LL, -1LL, -1LL, -1LL};
  size_type n_v = (size_type)1LL;
  value_type v_v = (value_type)-536870913LL;
  long long r = (long long)(find(a_m, n_v, v_v));
  printf("RES %lld\n", r);
  for (int i = 0; i < 4; i++) printf("MEM a %d %lld\n", i, (long long)a_m[i]);
  return 0;
}
