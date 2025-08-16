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
DEFINE_CODEC(string) {
	.type = codec_primitive,
	.name = "string",
	.type_size = sizeof(string),
	.mutable = true,
	.free = (codec_free_fn)string_free,
	.primitive_data = {
		.type = codec_primitive_string,
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
DEFINE_CODEC(str) {
	.type = codec_primitive,
	.name = "str",
	.type_size = sizeof(str),
	.mutable = false,
	.free = nullptr,
	.primitive_data = {
		.type = codec_primitive_string,
		.to_string = str_to_string,
		.from_string = str_from_string,
	},
};

string string_new(const char* str) {
	usize len = strlen(str);
	usize cap = len + 1; // Account for the null-terminator.

	char* data = nil_alloc(cap);
	memcpy(data, str, cap);

	return (string){
		.data = data,
		.len = len,
		.cap = cap,
	};
}

void string_free(string* str) {
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
