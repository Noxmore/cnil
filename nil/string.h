#pragma once

#include "nint.h"

#include <bits/types/FILE.h>

// Heap-allocated mutable string type.
typedef struct string {
	/// A null terminated c string.
	char* data;
	// Length of the string, not including the null terminator.
	usize len;
	// Length of the underlying buffer, including the null terminator.
	usize cap;
} string;
// To make some syntax nicer, like returning an empty string from a function.
constexpr string EMPTY_STRING = { 0 };

// String slice. Does not own the data contained within. Treat like a fat pointer.
typedef struct str {
	// Immutable data of the string.
	// You should not assume that this is null-terminated unless explicitly told that it is.
	const char* data;
	usize len;
} str;
// To make some syntax nicer, like returning an empty string from a function.
constexpr str EMPTY_STR = { 0 };

// Creates a static `str` slice from a string literal.
#define s(LITERAL) (str){ .data = LITERAL, .len = sizeof(LITERAL)-1 }

// Creates a string via a null terminated C string.
string string_new(const char* str);
void string_free(string* str);
/// Creates a new string with regular C formatting codes.
string string_format(const char* fmt, ...) __attribute__((format(printf, 1, 2)));
// Allocates a new string with the same contents as `s`. Does not take ownership.
string string_clone(string s);
str string_as_slice(const string* s);
// Reads a string from a file, preallocating exactly how much data the file contains.
// If any step of this process fails, returns an empty/null string.
string read_string_from_file(FILE* file);

// Creates a string slice from a null terminated C string. Does not allocate anything.
// Do not call this in a string literal. Use the `s` macro instead, as it does not need to call strlen.
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
// Version of `str_eq` that compares to a c string for ease of use, but is probably slightly slower.
bool str_eq_cstr(str a, const char* b);

// Little helper function that writes a `str` out to a file.
void str_write(str s, FILE* file);

// -- STRING UTILITIES -- //

const char* cstr_get_filename(const char* path);

bool cstr_starts_with(const char* s, const char* prefix);
bool str_starts_with(str s, str prefix);

void str_get_row_col(str s, usize pos, usize* row, usize* col);

typedef struct nil_string_splitter {
	const char* src;
	const char* pattern;
	const char* cursor;
} nil_string_splitter;

nil_string_splitter nil_split_cstr(const char* s, const char* pattern);
bool nil_split_next(nil_string_splitter* splitter, str* substring);