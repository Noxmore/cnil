#pragma once

#include "macro_utils.h"

void $_nil_panic(const char* fn, const char* at, const char* fmt, ...);

#define panic($fmt, ...) $_nil_panic(__FUNCTION__, __FILE_NAME__ ":" NIL_STRINGIFY(__LINE__), $fmt, ## __VA_ARGS__)

#define UNREACHABLE panic("BUG: This should be unreachable!")

// TODO: asserts, panic hooks
// #define assert($cond, $fmt, ...) if (!($cond)) panic($fmt, __VA_ARGS__)