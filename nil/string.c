#include "string.h"

#include "math.h"

#include <assert.h>
#include <memory.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <sys/stat.h>


string string_new(const char* str, allocator_ref allocator) {
	if (str == nullptr)
		return EMPTY_STRING;

	const usize len = strlen(str);

	if (len == 0)
		return EMPTY_STRING;

	const usize cap = len + 1; // Account for the null-terminator.

	char* data = allocator.alloc(allocator.ctx, nullptr, alignof(char), 0, cap);
	memcpy(data, str, cap);

	return (string){
		.data = data,
		.len = len,
		.cap = cap,
	};
}

string string_sized(const usize len, allocator_ref allocator) {
	const usize cap = len + 1; // Account for the null-terminator.
	char* data = allocator.alloc(allocator.ctx, nullptr, alignof(char), 0, cap);
	data[len] = '\0';

	return (string){
		.data = data,
		.len = len,
		.cap = cap,
	};
}

string string_concat_array(allocator_ref allocator, const usize str_count, const str strs[static str_count]) {
	usize total_len = 0;

	for (usize i = 0; i < str_count; i++)
		total_len += strs[i].len;

	string s = string_sized(total_len, allocator);

	char* cursor = s.data;

	for (usize i = 0; i < str_count; i++) {
		const usize len = strs[i].len;
		memcpy(cursor, strs[i].data, len);
		cursor += len;
	}

	return s;
}

void string_free(string str, allocator_ref allocator) {
	allocator.alloc(allocator.ctx, str.data, alignof(char), str.cap, 0);
}

string string_format(allocator_ref allocator, const char* fmt, ...) {
	va_list args;
	va_start(args, fmt);
	const int len = vsnprintf(nullptr, 0, fmt, args);
	va_end(args);

	if (len < 0)
		return EMPTY_STRING;

	string out = string_sized(len, allocator);

	va_start(args, fmt);
	vsnprintf(out.data, out.cap, fmt, args);
	va_end(args);

	return out;
}

string string_clone(const string* s, allocator_ref allocator) {
	string cloned = string_sized(s->len, allocator);
	memcpy(cloned.data, s->data, s->cap);
	return cloned;
}

str string_as_slice(const string* s) {
	return (str) {
		.data = s->data,
		.len = s->len,
	};
}

string read_string_from_file(FILE* file, allocator_ref allocator) {
	struct stat file_stats;
	if (fstat(fileno(file), &file_stats) != 0 || !S_ISREG(file_stats.st_mode))
		return EMPTY_STRING;

	string s = string_sized(file_stats.st_size, allocator);

	fread(s.data, file_stats.st_size, 1, file);

	// Length will be determined after reading just in case the file has a null byte. Alternatively, we could replace all \0 chars with 0 or something.
	s.len = strlen(s.data);

	return s;
}

str str_new(const char* s) {
	return (str){
		.data = s,
		.len = strlen(s),
	};
}

string str_allocate(const str s, allocator_ref allocator) {
	string owned = string_sized(s.len, allocator);
	memcpy(owned.data, s.data, s.len);

	return owned;
}

bool str_is_cstr(const str s) {
	return s.data[s.len] == 0;
}

str str_slice(const str s, const usize from, const usize to) {
	if (to <= from)
		return EMPTY_STR;
	return str_slice_from(str_slice_to(s, to), from);
	// const usize from_offset = min_usize(from, s.len);
	// return (str){ .data = s.data + from_offset, .len = s.len - from_offset };
}
str str_slice_to(const str s, const usize to) {
	return (str){ .data = s.data, .len = min_usize(s.len, to) };
}
str str_slice_from(const str s, const usize from) {
	const usize offset = min_usize(from, s.len);
	return (str){ .data = s.data + offset, .len = s.len - offset };
}

bool str_eq(const str a, const str b) {
	return
			a.len == b.len &&
			(
				// This covers nullptr == nullptr.
				a.data == b.data ||
				// Make sure neither are nullptr, as passing nullptr into memcmp is undefined behavior.
				(a.data && b.data && memcmp(a.data, b.data, a.len) == 0)
			)
		;
}

bool str_eq_cstr(const str a, const char* b) {
	return strncmp(a.data, b, a.len) == 0;
}
void str_write(const str s, FILE* file) {
	fwrite(s.data, s.len, 1, file);
}

// -- STRING UTILITIES -- //

const char* cstr_get_filename(const char* path) {
	const char* previous_slash = path;

	while (*path != '\0') {
		if (*path == '/')
			previous_slash = path;
		path++;
	}

	return previous_slash+1;
}

bool cstr_starts_with(const char* s, const char* prefix) {
	return strncmp(prefix, s, strlen(prefix)) == 0;
}
bool str_starts_with(const str s, const str prefix) {
	return s.len >= prefix.len && memcmp(s.data, prefix.data, prefix.len) == 0;
}
void str_get_row_col(const str s, const usize pos, usize* row_out, usize* col_out) {
	if (row_out) *row_out = 1;
	if (col_out) *col_out = 1;
	usize row = 1;
	usize col = 1;

	if (pos >= s.len)
		return;
	for (usize i = 0; i < pos; i++) {
		if (s.data[i] == '\n') {
			col = 1;
			row++;
		} else {
			col++;
		}
	}

	if (row_out) *row_out = row;
	if (col_out) *col_out = col;
}

nil_string_splitter nil_split_cstr(const char* s, const char* pattern) {
	assert(s != nullptr);
	assert(pattern != nullptr);
	return (nil_string_splitter){ .src = s, .pattern = pattern, .cursor = s };
}
bool nil_split_next(nil_string_splitter* splitter, str* substring) {
	// If we've reached the end, there's no more splits to be had.
	if (*splitter->cursor == '\0')
		return false;

	// If the pattern has zero length, we return the entire string.
	if (*splitter->pattern == '\0') {
		*substring = str_new(splitter->src);
		splitter->cursor = "";
		return true;
	}

	const char* starting_cursor = splitter->cursor;
	usize i = 0;
	for (; *splitter->cursor != '\0'; splitter->cursor++, i++) {
		// Match against the pattern.
		usize progress = 0;
		while (splitter->cursor[progress] != '\0' && splitter->cursor[progress] == splitter->pattern[progress]) {
			progress++;
			if (splitter->pattern[progress] == '\0') {
				splitter->cursor += progress;
				*substring = (str){ .data = starting_cursor, .len = i };
				return true;
			}
		}
	}

	// We got to the end, let's return the remainder.
	*substring = (str){ .data = starting_cursor, .len = i };
	return true;
}