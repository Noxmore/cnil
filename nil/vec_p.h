// Work-in-progress alternate implementation of growable arrays where the metadata is stored in the place the pointer is stored at.

#pragma once

#include "nint.h"

#define vec(T) T*

typedef void erased_vec;

// Growable heap array metadata stored before the vector's data.
typedef struct vec_meta {
	usize len;
	usize cap;
	usize type_size;
} vec_meta;

#define vec_push($vec, $v) *(typeof(*($vec)))internal_vec_reserve_item((erased_vec**)$vec, sizeof(**($vec))) = $v
#define vec_free($vec) internal_vec_free((erased_vec**)$vec);

vec_meta* vec_info(erased_vec* vec);
usize vec_len(erased_vec* vec);
usize vec_cap(erased_vec* vec);
void internal_vec_free(erased_vec** vec);
void* internal_vec_reserve_item(erased_vec** vec, usize type_size);
void vec_pop(erased_vec* vec, void* dst);
void internal_vec_reallocate(erased_vec** vec, usize cap, usize type_size);
