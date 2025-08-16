#pragma once

#include "nint.h"

typedef struct slice {
	void* const data;
	const u32 len;
	const u32 block_size;
} slice;

#define slice_type(T) struct slice$##T
#define slice(T) slice_type(T) { T* const data; const u32 len; const u32 block_size; }
// #define slice_new(T, $ptr, $len)
// #define slice_stack(T, $n) (slice(T)){ .data = (T[]){} }
#define subslice($slice, $from, $to) (slice(typeof(*($slice).data)))internal_subslice((slice*)($slice), $from, $to)

// From is inclusive, and to is exclusive.
slice internal_subslice(slice slice, u32 from, u32 to);