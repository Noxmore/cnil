#include "codec.h"

#include "ansi_colors.h"

#include <memory.h>
#include <stdio.h>
#include <stdlib.h>

#define COLOR_COMMENT ANSI_COLOR_BLACK
#define COLOR_KEYWORD ANSI_COLOR_RED
#define COLOR_NUMBER ANSI_COLOR_MAGENTA
#define COLOR_STRING ANSI_COLOR_GREEN
#define COLOR_TYPE ANSI_COLOR_YELLOW
#define COLOR_ENUM ANSI_COLOR_CYAN

const char* codec_enum_name_from_variant_value(const codec* codec, s64 variant_value) {
	if (codec == nullptr) return nullptr;
	if (codec->type != codec_enum) return nullptr;

	for (usize i = 0; i < codec->enum_data.variant_count; i++) {
		if (variant_value == codec->enum_data.variant_values[i])
			return codec->enum_data.variant_names[i];
	}

	return nullptr;
}

static void indent(u32 depth) {
	for (u32 i = 0; i < depth; i++)
		printf("  ");
}

static s64 get_any_sized_int(const void* data, u8 type_size) {
	s64 value = 0;
	memcpy(&value, data, type_size);
	// Move the transferred bits to the right of the number.
	// UPDATE: Apparently it is already aligned correctly?
	// value >>= (s64)(sizeof(s64) - type_size);
	return value;
}

static void debug_with_codec_recursive(const void* obj, const codec* codec, const u32 depth) {
	if (codec == nullptr) {
		printf(ANSI_COLOR_RED "<null codec>");
		return;
	}

	switch (codec->type) {
		case codec_struct:
			if (codec->name != nullptr) {
				printf(COLOR_KEYWORD "struct " COLOR_TYPE "%s " ANSI_COLOR_RESET "{\n", codec->name);
			} else {
				puts(COLOR_KEYWORD "struct" ANSI_COLOR_RESET " {");
			}

			for (usize i = 0; i < codec->struct_data.field_count; i++) {
				indent(depth + 1);
				const codec_field* field = &codec->struct_data.fields[i];
				printf(".%s = ", field->name);

				const void* field_ptr = obj + field->struct_offset;

				switch (field->type) {
					case codec_field_value:
						debug_with_codec_recursive(field_ptr, field->sub_codec, depth + 1);
						break;
					case codec_field_pointer:
						debug_with_codec_recursive(*(void**)field_ptr, field->sub_codec, depth + 1);
						break;
					default:
						printf(ANSI_COLOR_RED "<invalid field type: %u>", field->type);
				}

				puts(ANSI_COLOR_RESET ",");
			}

			indent(depth);
			puts("}");

			break;
		case codec_enum:
			const s64 value = get_any_sized_int(obj, codec->type_size);
			const char* name = codec_enum_name_from_variant_value(codec, value);
			if (name != nullptr)
				printf(COLOR_ENUM "%s " COLOR_COMMENT "/* %li */", name, value);
			else
				printf(ANSI_COLOR_RED "<invalid variant value: %li>", value);
			break;
		case codec_primitive:
			if (codec->primitive_data.type == codec_primitive_sint || codec->primitive_data.type == codec_primitive_uint || codec->primitive_data.type == codec_primitive_real)
				fputs(COLOR_NUMBER, stdout);
			else if (codec->primitive_data.type == codec_primitive_string)
				fputs(COLOR_STRING "\"", stdout);

			codec->primitive_data.to_string(stdout, obj);

			if (codec->primitive_data.type == codec_primitive_string)
				fputc('\"', stdout);
			break;
		default:
			printf(ANSI_COLOR_RED "<invalid type: %u>", codec->type);
	}
}

void debug_with_codec(const void* obj, const codec* codec) {
	debug_with_codec_recursive(obj, codec, 0);
}

#define INTEGER_CODEC(T, $fmt, $type)                                          \
	static void T##_to_string(FILE* file, const void* ptr) {                    \
		fprintf(file, $fmt, *(T*)ptr);                                           \
	}                                                                           \
	static void T##_from_string(const char* src, char** end_ptr, void* dst) {   \
		*(T*)dst = strtol(src, end_ptr, 10);                                     \
	}                                                                           \
	DEFINE_CODEC(T) {                                                           \
		.type = codec_primitive,                                                 \
		.name = #T,                                                              \
		.type_size = sizeof(T),                                                  \
		.mutable = true,                                                         \
		.free = nullptr,                                                         \
		.primitive_data = {                                                      \
			.type = $type,                                                        \
			.to_string = T##_to_string,                                           \
			.from_string = T##_from_string,                                       \
		},                                                                       \
	};

INTEGER_CODEC(u8, "%u", codec_primitive_uint)
INTEGER_CODEC(u16, "%u", codec_primitive_uint)
INTEGER_CODEC(u32, "%u", codec_primitive_uint)
INTEGER_CODEC(u64, "%lu", codec_primitive_uint)
INTEGER_CODEC(usize, "%lu", codec_primitive_uint)

INTEGER_CODEC(uint, "%u", codec_primitive_uint)
INTEGER_CODEC(ushort, "%u", codec_primitive_uint)
INTEGER_CODEC(ulong, "%lu", codec_primitive_uint)

INTEGER_CODEC(size_t, "%lu", codec_primitive_uint)

INTEGER_CODEC(s8, "%i", codec_primitive_sint)
INTEGER_CODEC(s16, "%i", codec_primitive_sint)
INTEGER_CODEC(s32, "%i", codec_primitive_sint)
INTEGER_CODEC(s64, "%li", codec_primitive_sint)

INTEGER_CODEC(int, "%i", codec_primitive_sint)
INTEGER_CODEC(short, "%i", codec_primitive_sint)
INTEGER_CODEC(long, "%li", codec_primitive_sint)

static void float_to_string(FILE* file, const void* ptr) {
	fprintf(file, "%f", *(float*)ptr);
}
static void float_from_string(const char* src, char** end_ptr, void* dst) {
	*(float*)dst = strtof(src, end_ptr);
}
DEFINE_CODEC(float) {
	.type = codec_primitive,
	.name = "float",
	.type_size = sizeof(float),
	.mutable = true,
	.free = nullptr,
	.primitive_data = {
		.type = codec_primitive_real,
		.to_string = float_to_string,
		.from_string = float_from_string,
	},
};

static void double_to_string(FILE* file, const void* ptr) {
	fprintf(file, "%f", *(double*)ptr);
}
static void double_from_string(const char* src, char** end_ptr, void* dst) {
	*(double*)dst = strtof(src, end_ptr);
}
DEFINE_CODEC(double) {
	.type = codec_primitive,
	.name = "double",
	.type_size = sizeof(double),
	.mutable = true,
	.free = nullptr,
	.primitive_data = {
		.type = codec_primitive_real,
		.to_string = double_to_string,
		.from_string = double_from_string,
	},
};