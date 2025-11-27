#pragma once

#include "nint.h"
#include "reflect.h"

// Heap-allocated growable string type. Vector manipulation functions work on this type.
typedef struct string {
	/// A null terminated c string.
	char* data;
	usize len;
	usize cap;
} string;
DECLARE_TYPE_INFO(string)

// String slice. Does not own the data contained within.
typedef struct str {
	const char* data;
	usize len;
} str;
DECLARE_TYPE_INFO(str)

// Creates a string via a null terminated C string.
string string_new(const char* str);
void string_free(string* str);
str string_as_slice(const string* s);

// Creates a string slice from a null terminated C string. Does not allocate anything.
str str_new(const char* s);
// Convert a string slice into an owned string.
string str_allocate(str s);
// Returns whether this string slice is null-terminated or not. Subslices probably won't be null-terminated for example.
bool str_is_cstr(str s);

// Slices a string slice. `from` is inclusive, `to` is exclusive. Both values will get clipped to `len`.
str str_slice(str s, usize from, usize to);
// Slices a string slice. `to` is exclusive, and will get clipped to `len`.
str str_slice_to(str s, usize to);
// Slices a string slice. `from` is inclusive, and will get clipped to `len`.
str str_slice_from(str s, usize from);

// Returns `true` if the contents of both strings are identical, else `false`.
bool str_eq(str a, str b);
bool str_eq_cstr(str a, const char* b);

// -- STRING UTILITIES -- //

const char* cstr_get_filename(const char* path);

bool cstr_starts_with(const char* s, const char* prefix);
bool str_starts_with(str s, str prefix);

typedef struct nil_string_splitter {
	const char* src;
	const char* pattern;
	const char* cursor;
} nil_string_splitter;

nil_string_splitter nil_split_string(const char* s, const char* pattern);
bool nil_split_next(nil_string_splitter* splitter, str* substring);