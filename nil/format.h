// WIP string formatting, not ready for use.

#pragma once

#include "string.h"
#include "macro_utils.h"
// #include "stream.h"

/*typedef void (*nil_formatter_fn)(write_stream*, void*);

void register_nil_formatter(const char* fmt_code, nil_formatter_fn formatter);

void format_str(write_stream* stream, const char* fmt, ...);*/

string nil_internal_concat_strings(usize string_count, ...);
// #define NIL_CONCAT_STRINGS_ARG_WRAPPER(X) _Generic((X), str: (X), string: string_as_slice(&(X)), char*: s(X))
#define concat_strs(...) nil_internal_concat_strings(NIL_COUNT_ARGS(__VA_ARGS__), __VA_ARGS__)