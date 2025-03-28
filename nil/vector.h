#pragma once

#include "nint.h"

#define N_VECTOR_MAGIC_NUMBER 0xF2F3

/*#ifndef N_VECTOR_IDENT
#define N_VECTOR_IDENT vec
#endif

#define N_VECTOR_CONCAT_EXPANDED(a, b) a##b
#define N_VECTOR_CONCAT(a, b) N_VECTOR_CONCAT_EXPANDED(a, b)
#define N_VECTOR_OPERATION(ident) N_VECTOR_CONCAT(N_VECTOR_IDENT, ident)*/

// Define N_VECTOR_SUPPRESS_ERRORS to not print errors to stderr

// #define vector(T) T*

typedef void* vector;

usize vecLen(vector vec);
usize vecCap(vector vec);
void vecDrop(vector vec);
// Sets the vector's len to 0. You need to manually drop all elements before doing this.
void vecClear(vector vec);

// USAGE: vecPush(vector* vecRef, T item)
#define vecPush(vec, item) *(typeof(**vec))internal_vecReserveItem(vec, sizeof(*vec)) = (item)
// Pops one item from the end of the vector.
// USAGE: vecPop(vector vec, T* dst)
#define vecPop(vec, dst) internal_vecPop(vec, sizeof(*vec), (typeof(*vec))dst)
// Copies data from an existing array, overriding current data.
// USAGE: vecCopyFrom(vector* vecRef, T* src, usize count)
#define vecCopyFrom(vec, src, count) internal_vecCopyFrom(vec, sizeof(*vec), (typeof(**vec))src, count)

void* internal_vecReserveItem(vector* vec, usize itemSize);
void internal_vecReallocate(vector* vec, usize itemSize, usize newCap);
void internal_vecPop(vector vec, usize itemSize, void* dst);
void internal_vecCopyFrom(vector* vec, usize itemSize, const void* src, usize count);
// bool internal_vecContains(vector vec, usize itemSize, void* value);