#pragma once

#include "macro_utils.h"
#include "string.h"

void $_nil_panic(const char* fn, const char* at, const char* fmt, ...)
	__attribute__((__noreturn__))
	__attribute__((__cold__))
	__attribute__((format(printf, 3, 4)));

typedef struct nil_panic_info {
	const char* fn;
	const char* at;
	string message;
} nil_panic_info;
typedef void (*nil_panic_hook)(nil_panic_info);
extern nil_panic_hook nil_current_panic_hook;

#define panic($fmt, ...) $_nil_panic(__FUNCTION__, __FILE_NAME__ ":" NIL_STRINGIFY(__LINE__), $fmt, ## __VA_ARGS__)

#ifdef __GNUC__
#define UNREACHABLE __builtin_unreachable()
#elifdef _MSC_VER
#define UNREACHABLE __assume(false)
#else
#define UNREACHABLE panic("BUG: This should be unreachable!")
#endif
