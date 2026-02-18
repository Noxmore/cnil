#pragma once

// This file is just to split the definition of `vec`s off so that we can use it in alloc.h

#include "../nint.h"
#include "../macro_utils.h"

// Type-erased vector. Mainly used internally, but can also be used for runtime-defined/runtime-sized types.
typedef struct erased_vec {
	void* data;
	usize len;
	usize cap;
} erased_vec;

#define NIL_VEC_MARKER "nil_vec_marker"

// Heap-allocated growable array.
#define vec_named(T, NAME) struct ANNOTATE(NIL_VEC_MARKER) vec_$_##NAME { T* data; usize len; usize cap; }
// Heap-allocated growable array.
#define vec(T) vec_named(T, T)
// Like `vec`, but doesn't specify a struct name.
// This can be useful when creating a vector of pointer types, but doesn't cover some cases that a named vector does. For those you should use `vec_named` or a typedef.
#define vec_anon(T) struct ANNOTATE(NIL_VEC_MARKER) { T* data; usize len; usize cap; }