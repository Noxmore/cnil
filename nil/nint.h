#pragma once

// I think these macros are GNU C only. We will need to change this if we want to support MSVC.

#include <bits/wordsize.h>

typedef __UINT8_TYPE__ u8;
typedef __UINT16_TYPE__ u16;
typedef __UINT32_TYPE__ u32;
typedef __UINT64_TYPE__ u64;
typedef unsigned _BitInt(128) u128;
typedef __SIZE_TYPE__ usize;
typedef __INT8_TYPE__ s8;
typedef __INT16_TYPE__ s16;
typedef __INT32_TYPE__ s32;
typedef __INT64_TYPE__ s64;
typedef _BitInt(128) s128;
typedef __PTRDIFF_TYPE__ ssize;



// Integer limits copied from stdint.

# if __WORDSIZE == 64
#  define NIL_INT64_C(c)	c ## L
#  define NIL_UINT64_C(c)	c ## UL
# else
#  define NIL_INT64_C(c)	c ## LL
#  define NIL_UINT64_C(c)	c ## ULL
# endif

/* Limits of integral types.  */

/* Minimum of signed integral types.  */
constexpr s8 S8_MIN = -128;
constexpr s16 S16_MIN = -32767-1;
constexpr s32 S32_MIN = -2147483647-1;
constexpr s64 S64_MIN = -NIL_INT64_C(9223372036854775807)-1;
/* Maximum of signed integral types.  */
constexpr s8 S8_MAX = 127;
constexpr s16 S16_MAX = 32767;
constexpr s32 S32_MAX = 2147483647;
constexpr s64 S64_MAX = NIL_INT64_C(9223372036854775807);

/* Maximum of unsigned integral types.  */
constexpr u8 U8_MAX = 255;
constexpr u16 U16_MAX = 65535;
constexpr u32 U32_MAX = 4294967295U;
constexpr u64 U64_MAX = NIL_UINT64_C(18446744073709551615);