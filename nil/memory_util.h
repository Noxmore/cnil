#pragma once

#include "nint.h"

inline static usize pad_to_align(const usize size, const usize align) {
	return (size + align - 1) & ~(align - 1);
}