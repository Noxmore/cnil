#pragma once

#include "nint.h"

// Handy little struct for keeping a type size and alignment together. This is mainly good for function parameters, so the user doesn't get them mixed up.
typedef struct type_layout {
	usize size;
	usize align;
} type_layout;

#define layoutof(EXPR) ((type_layout){ .size = sizeof(EXPR), .align = alignof(typeof(EXPR)) })
/*#define array_layoutof(EXPR, N) ((type_layout){ .size = sizeof(EXPR) * N, .align = alignof(typeof(EXPR)) })

// Multiplies `layout.size` by `size` and returns the resulting layout.
static inline type_layout array_layout(type_layout layout, const usize size) {
	layout.size *= size;
	return layout;
}*/