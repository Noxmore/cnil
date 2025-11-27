#include "string.h"

#include "math.h"

#include <assert.h>
#include <memory.h>
#include <stdio.h>
#include <stdlib.h>

static void string_from_string(const char* src, char** end_ptr, void* dst) {
	*(string*)dst = string_new(src);
}
static void string_to_string(FILE* file, const void* src) {
	fputs(((string*)src)->data, file);
}
DEFINE_TYPE_INFO(string,
	.kind = type_info_opaque,
	.mutable = true,
	.name = "string",
	.size = sizeof(string),
	.align = alignof(string),
	.annotations = nullptr,
	.annotation_count = 0,
	.free = (type_info_free_fn)string_free,
	.opaque_data = {
		.kind = type_info_opaque_string,
		.to_string = string_to_string,
		.from_string = string_from_string,
	},
)
// REFLECT_FREE_FN(string, string_free)

static void str_from_string(const char* src, char** end_ptr, void* dst) {
	// TODO: This doesn't take ownership, should this function be nullptr?
	*(str*)dst = str_new(src);
}
static void str_to_string(FILE* file, const void* src) {
	fputs(((str*)src)->data, file);
}
DEFINE_TYPE_INFO(str,
	.kind = type_info_opaque,
	.mutable = false,
	.name = "str",
	.size = sizeof(str),
	.align = alignof(str),
	.free = nullptr,
	.opaque_data = {
		.kind = type_info_opaque_string,
		.to_string = str_to_string,
		.from_string = str_from_string,
	},
)

string string_new(const char* str) {
	if (str == nullptr)
		return (string){0};

	const usize len = strlen(str);

	if (len == 0)
		return (string){0};

	const usize cap = len + 1; // Account for the null-terminator.

	char* data = malloc(cap);
	memcpy(data, str, cap);

	return (string){
		.data = data,
		.len = len,
		.cap = cap,
	};
}

void string_free(string* str) {
	if (str->data)
		free(str->data);
	*str = (string) { 0 };
}

str string_as_slice(const string* s) {
	return (str) {
		.data = s->data,
		.len = s->len,
	};
}

str str_new(const char* s) {
	return (str){
		.data = s,
		.len = strlen(s),
	};
}

string str_allocate(const str s) {
	char* buf = malloc(s.len + 1); // Account for null-terminator.
	memcpy(buf, s.data, s.len);

	// Make sure null terminator is set.
	buf[s.len] = 0;

	return (string) {
		.data = buf,
		.len = s.len,
		.cap = s.len + 1,
	};
}

bool str_is_cstr(const str s) {
	return s.data[s.len] == 0;
}

str str_slice(const str s, const usize from, const usize to) {
	if (to <= from)
		return (str){0};
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
	return a.len == b.len && memcmp(a.data, b.data, a.len) == 0;
}

bool str_eq_cstr(const str a, const char* b) {
	return strncmp(a.data, b, a.len) == 0;
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

nil_string_splitter nil_split_string(const char* s, const char* pattern) {
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