#pragma once

#include "nint.h"
#include "codec.h"

// Heap-allocated growable string type. Interface with like a vector.
typedef struct string {
	/// A null terminated c string.
	char* data;
	usize len;
	usize cap;
} string;
EXTERN_CODEC(string)

// String slice.
typedef struct str {
	const char* data;
	usize len;
} str;
EXTERN_CODEC(str)

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