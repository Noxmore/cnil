#pragma once

#include "nint.h"

#define NIL_BASIC_NUM_FNS(T) \
	static inline T min_##T(T a, T b) { return a < b ? a : b; } \
	static inline T max_##T(T a, T b) { return a > b ? a : b; } \
	static inline T clamp_##T(T v, T min, T max) { return min_##T(max_##T(v, min), max); }

#define NIL_BASIC_INT_FNS(T) \
	NIL_BASIC_NUM_FNS(T) \
	static inline T sat_add_##T(T a, T b) { T res = a + b; res |= -(res < a); return res; } \
	static inline T sat_sub_##T(T a, T b) { T res = a - b; res &= -(res <= a); return res; }

NIL_BASIC_INT_FNS(u8)
NIL_BASIC_INT_FNS(u16)
NIL_BASIC_INT_FNS(u32)
NIL_BASIC_INT_FNS(u64)
NIL_BASIC_INT_FNS(u128)
NIL_BASIC_INT_FNS(usize)

NIL_BASIC_INT_FNS(s8)
NIL_BASIC_INT_FNS(s16)
NIL_BASIC_INT_FNS(s32)
NIL_BASIC_INT_FNS(s64)
NIL_BASIC_INT_FNS(s128)

NIL_BASIC_NUM_FNS(float)
NIL_BASIC_NUM_FNS(double)

/*#define min($a, $b) _Generic(($a), \
	u8: min_u8($a, $b),             \
	u16: min_u16($a, $b),           \
	u32: min_u32($a, $b),           \
	u64: min_u64($a, $b),           \
	s8: min_s8($a, $b),             \
	s16: min_s16($a, $b),           \
	s32: min_s32($a, $b),           \
	s64: min_s64($a, $b),           \
	float: min_float($a, $b),       \
	double: min_double($a, $b)      \
)

#define max($a, $b) _Generic(($a), \
	u8: max_u8($a, $b),             \
	u16: max_u16($a, $b),           \
	u32: max_u32($a, $b),           \
	u64: max_u64($a, $b),           \
	s8: max_s8($a, $b),             \
	s16: max_s16($a, $b),           \
	s32: max_s32($a, $b),           \
	s64: max_s64($a, $b),           \
	float: max_float($a, $b),       \
	double: max_double($a, $b)      \
)

#define clamp($v, $min, $max) _Generic(($v), \
	u8: clamp_u8($v, $min, $max),             \
	u16: clamp_u16($v, $min, $max),           \
	u32: clamp_u32($v, $min, $max),           \
	u64: clamp_u64($v, $min, $max),           \
	s8: clamp_s8($v, $min, $max),             \
	s16: clamp_s16($v, $min, $max),           \
	s32: clamp_s32($v, $min, $max),           \
	s64: clamp_s64($v, $min, $max),           \
	float: clamp_float($v, $min, $max),       \
	double: clamp_double($v, $min, $max)      \
)*/

#undef NIL_BASIC_NUM_FNS
#undef NIL_BASIC_INT_FNS