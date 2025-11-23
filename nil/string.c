#include "string.h"
#include "allocator.h"

#include <stdio.h>
#include <memory.h>

static void string_from_string(const char* src, char** end_ptr, void* dst) {
	*(string*)dst = string_new(src);
}
static void string_to_string(FILE* file, const void* src) {
	fputs(((string*)src)->data, file);
}
DEFINE_TYPE_INFO(string) {
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
};

static void str_from_string(const char* src, char** end_ptr, void* dst) {
	// TODO: This doesn't take ownership, should this function be nullptr?
	*(str*)dst = str_new(src);
}
static void str_to_string(FILE* file, const void* src) {
	fputs(((str*)src)->data, file);
}
DEFINE_TYPE_INFO(str) {
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
};

string string_new(const char* str) {
	const usize len = strlen(str);

	if (len == 0)
		return (string){0};

	const usize cap = len + 1; // Account for the null-terminator.

	char* data = nil_alloc(cap);
	memcpy(data, str, cap);

	return (string){
		.data = data,
		.len = len,
		.cap = cap,
	};
}

void string_free(string* str) {
	if (str->data)
		nil_free(str->data);
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
	char* buf = nil_alloc(s.len + 1); // Account for null-terminator.
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
