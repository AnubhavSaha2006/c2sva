/*
 * Task 438 Natural Language:
 * Write an ACSL-annotated C header file that defines integer types (signed and unsigned 8, 16, 32,
 * 64-bit, size_t, index_t) and comparison functions for each type. The comparison functions take
 * two integers and return an enumeration value (Lt, Eq, Gt) indicating whether the first is less
 * than, equal to, or greater than the second. Each comparison function has complete and disjoint
 * behaviors covering all cases.
 */

#ifndef BASETYPE_H
#define BASETYPE_H

#include <stddef.h>

// this is just renaming for types

typedef unsigned long size_t; // forward declaration

/////////////////////////////////////////////////
// integers
// format [s|u]int[<size in bits>]

typedef signed char sint8_t;
typedef unsigned char uint8_t;
typedef short sint16_t;
typedef unsigned short uint16_t;
typedef int sint32_t;
typedef unsigned int uint32_t;
typedef long long sint64_t;
typedef unsigned long long uint64_t;

typedef int sint_t;
typedef unsigned int uint_t;


/*
  logic size_t size_min(size_t s1, size_t s2) = s1 < s2 ? s1 : s2;

  lemma size_min_sem: \forall size_t s1, size_t s2; size_min(s1, s2) <= s1 && size_min(s1, s2) <= s2 && (size_min(s1, s2) == s1 || size_min(s1, s2) == s2);

*/

typedef uint_t index_t;

// redefine the type for pointers
typedef char* any_ptr_t;


typedef enum {Lt, Eq, Gt} cmp_t;

/*@
  assigns \nothing;
  
  behavior lt:
    assumes i1 < i2;
    ensures \result == Lt;
  
  behavior eq:
    assumes i1 == i2;
    ensures \result == Eq;
  
  behavior gt:
    assumes i1 > i2;
    ensures \result == Gt;
  
  complete behaviors lt, eq, gt;
  disjoint behaviors lt, eq, gt;
  
*/
cmp_t uint8_cmp(const uint8_t i1, const uint8_t i2);

/*@
  assigns \nothing;
  
  behavior lt:
    assumes i1 < i2;
    ensures \result == Lt;
  
  behavior eq:
    assumes i1 == i2;
    ensures \result == Eq;
  
  behavior gt:
    assumes i1 > i2;
    ensures \result == Gt;
  
  complete behaviors lt, eq, gt;
  disjoint behaviors lt, eq, gt;
  
*/
cmp_t sint8_cmp(const sint8_t i1, const sint8_t i2);

/*@
  assigns \nothing;
  
  behavior lt:
    assumes i1 < i2;
    ensures \result == Lt;
  
  behavior eq:
    assumes i1 == i2;
    ensures \result == Eq;
  
  behavior gt:
    assumes i1 > i2;
    ensures \result == Gt;
  
  complete behaviors lt, eq, gt;
  disjoint behaviors lt, eq, gt;
  
*/
cmp_t uint16_cmp(const uint16_t i1, const uint16_t i2);

/*@
  assigns \nothing;
  
  behavior lt:
    assumes i1 < i2;
    ensures \result == Lt;
  
  behavior eq:
    assumes i1 == i2;
    ensures \result == Eq;
  
  behavior gt:
    assumes i1 > i2;
    ensures \result == Gt;
  
  complete behaviors lt, eq, gt;
  disjoint behaviors lt, eq, gt;
  
*/
cmp_t sint16_cmp(const sint16_t i1, const sint16_t i2);


/*@
  assigns \nothing;
  
  behavior lt:
    assumes i1 < i2;
    ensures \result == Lt;

  behavior eq:
    assumes i1 == i2;
    ensures \result == Eq;

  behavior gt:
    assumes i1 > i2;
    ensures \result == Gt;

  complete behaviors lt, eq, gt;
  disjoint behaviors lt, eq, gt;

*/
cmp_t uint32_cmp(const uint32_t i1, const uint32_t i2);

/*@
  assigns \nothing;
  
  behavior lt:
    assumes i1 < i2;
    ensures \result == Lt;

  behavior eq:
    assumes i1 == i2;
    ensures \result == Eq;

  behavior gt:
    assumes i1 > i2;
    ensures \result == Gt;

  complete behaviors lt, eq, gt;
  disjoint behaviors lt, eq, gt;

*/
cmp_t sint32_cmp(const sint32_t i1, const sint32_t i2);


/*@
  assigns \nothing;
  
  behavior lt:
    assumes i1 < i2;
    ensures \result == Lt;

  behavior eq:
    assumes i1 == i2;
    ensures \result == Eq;

  behavior gt:
    assumes i1 > i2;
    ensures \result == Gt;

  complete behaviors lt, eq, gt;
  disjoint behaviors lt, eq, gt;

*/
cmp_t uint64_cmp(const uint64_t i1, const uint64_t i2);

/*@
  assigns \nothing;
  
  behavior lt:
    assumes i1 < i2;
    ensures \result == Lt;

  behavior eq:
    assumes i1 == i2;
    ensures \result == Eq;

  behavior gt:
    assumes i1 > i2;
    ensures \result == Gt;

  complete behaviors lt, eq, gt;
  disjoint behaviors lt, eq, gt;

*/
cmp_t sint64_cmp(const sint64_t i1, const sint64_t i2);


/*@
  assigns \nothing;
  
  behavior lt:
    assumes i1 < i2;
    ensures \result == Lt;

  behavior eq:
    assumes i1 == i2;
    ensures \result == Eq;

  behavior gt:
    assumes i1 > i2;
    ensures \result == Gt;

  complete behaviors lt, eq, gt;
  disjoint behaviors lt, eq, gt;

*/
cmp_t uint_cmp(const uint_t i1, const uint_t i2);

/*@
  assigns \nothing;
  
  behavior lt:
    assumes i1 < i2;
    ensures \result == Lt;

  behavior eq:
    assumes i1 == i2;
    ensures \result == Eq;

  behavior gt:
    assumes i1 > i2;
    ensures \result == Gt;

  complete behaviors lt, eq, gt;
  disjoint behaviors lt, eq, gt;

*/
cmp_t sint_cmp(const sint_t i1, const sint_t i2);


/*@
  assigns \nothing;
  
  behavior lt:
    assumes i1 < i2;
    ensures \result == Lt;

  behavior eq:
    assumes i1 == i2;
    ensures \result == Eq;

  behavior gt:
    assumes i1 > i2;
    ensures \result == Gt;

  complete behaviors lt, eq, gt;
  disjoint behaviors lt, eq, gt;

*/
cmp_t size_cmp(const size_t i1, const size_t i2);


/*@
  assigns \nothing;
  
  behavior lt:
    assumes i1 < i2;
    ensures \result == Lt;

  behavior eq:
    assumes i1 == i2;
    ensures \result == Eq;

  behavior gt:
    assumes i1 > i2;
    ensures \result == Gt;

  complete behaviors lt, eq, gt;
  disjoint behaviors lt, eq, gt;

*/
cmp_t index_cmp(const index_t i1, const index_t i2);



////////////////////////////////////////////////
// bool

typedef enum {True, False} bool_t;

#endif

// Implementations (required for verification to succeed)
cmp_t uint8_cmp(const uint8_t i1, const uint8_t i2) { if (i1 < i2) return Lt; else if (i1 > i2) return Gt; else return Eq; }
cmp_t sint8_cmp(const sint8_t i1, const sint8_t i2) { if (i1 < i2) return Lt; else if (i1 > i2) return Gt; else return Eq; }
cmp_t uint16_cmp(const uint16_t i1, const uint16_t i2) { if (i1 < i2) return Lt; else if (i1 > i2) return Gt; else return Eq; }
cmp_t sint16_cmp(const sint16_t i1, const sint16_t i2) { if (i1 < i2) return Lt; else if (i1 > i2) return Gt; else return Eq; }
cmp_t uint32_cmp(const uint32_t i1, const uint32_t i2) { if (i1 < i2) return Lt; else if (i1 > i2) return Gt; else return Eq; }
cmp_t sint32_cmp(const sint32_t i1, const sint32_t i2) { if (i1 < i2) return Lt; else if (i1 > i2) return Gt; else return Eq; }
cmp_t uint64_cmp(const uint64_t i1, const uint64_t i2) { if (i1 < i2) return Lt; else if (i1 > i2) return Gt; else return Eq; }
cmp_t sint64_cmp(const sint64_t i1, const sint64_t i2) { if (i1 < i2) return Lt; else if (i1 > i2) return Gt; else return Eq; }
cmp_t uint_cmp(const uint_t i1, const uint_t i2) { if (i1 < i2) return Lt; else if (i1 > i2) return Gt; else return Eq; }
cmp_t sint_cmp(const sint_t i1, const sint_t i2) { if (i1 < i2) return Lt; else if (i1 > i2) return Gt; else return Eq; }
cmp_t size_cmp(const size_t i1, const size_t i2) { if (i1 < i2) return Lt; else if (i1 > i2) return Gt; else return Eq; }
cmp_t index_cmp(const index_t i1, const index_t i2) { if (i1 < i2) return Lt; else if (i1 > i2) return Gt; else return Eq; }