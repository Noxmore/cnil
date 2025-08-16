#pragma once

#include "nint.h"
#include "macro_utils.h"

#include <bits/types/FILE.h>
// #include <stddef.h>

typedef struct codec codec;

typedef struct codec_field {
	const char* name;

	enum codec_field_type : u8 {
		codec_field_value,
		codec_field_pointer,
		codec_field_type_count,
	} type;

	u16 len;
	u16 struct_offset;

	const codec* sub_codec;
} codec_field;

typedef void (*codec_free_fn)(void*);

// A codec describes an in-memory data structure, providing a form of reflection. This is particularly useful for serialization.
typedef struct codec {
	enum codec_type : u8 {
		codec_struct,
		codec_enum,
		codec_primitive,
		codec_type_count,
	} type;

	// Optional type name. Can be nullptr.
	const char* name;
	usize type_size;
	bool mutable;

	// Custom free function. Can be nullptr.
	codec_free_fn free;

	union {
		struct {
			usize field_count;
			codec_field* fields;
		} struct_data;

		struct {
			usize variant_count;
			const char** variant_names;
			s64* variant_values;
		} enum_data;

		struct {
			enum codec_primitive_type : u8 {
				codec_primitive_other,
				codec_primitive_sint,
				codec_primitive_uint,
				codec_primitive_real,
				codec_primitive_string,
				codec_primitive_type_count,
			} type;
			void (*to_string)(FILE*, const void*);
			void (*from_string)(const char* src, char** end_ptr, void* dst);
		} primitive_data;
	};
} codec;

#define PRIMITIVE_CODEC(T)

#define TYPE_CODEC_NAME(T) T##_$_codec
#define EXTERN_CODEC(T) extern const codec TYPE_CODEC_NAME(T);
#define DEFINE_CODEC(T) const codec TYPE_CODEC_NAME(T) =

EXTERN_CODEC(u8)
EXTERN_CODEC(u16)
EXTERN_CODEC(u32)
EXTERN_CODEC(u64)
EXTERN_CODEC(usize)

EXTERN_CODEC(uint)
EXTERN_CODEC(ushort)
EXTERN_CODEC(ulong)

EXTERN_CODEC(size_t)

EXTERN_CODEC(s8)
EXTERN_CODEC(s16)
EXTERN_CODEC(s32)
EXTERN_CODEC(s64)

EXTERN_CODEC(int)
EXTERN_CODEC(short)
EXTERN_CODEC(long)

EXTERN_CODEC(float)
EXTERN_CODEC(double)

#define ENUM_CODEC(T, ...) DEFINE_CODEC(T) {                         \
	.type = codec_enum,                                                                \
	.name = #T,                                                                        \
	.type_size = sizeof(enum T),                                                       \
	.mutable = true,                                                                   \
	.free = nullptr,                                                                   \
	.enum_data = {                                                                     \
		.variant_count = NIL_COUNT_ARGS(__VA_ARGS__),                                   \
		.variant_names = (const char*[]){NIL_WRAP_VA_ARGS(NIL_STRINGIFY, __VA_ARGS__)}, \
		.variant_values = (s64[]){__VA_ARGS__},                                         \
	},                                                                                 \
};

/*#define NIL_STRUCT_CODEC_FIELD_TYPE($field_type) _Generic(($field_type),\
	s8: codec_field_sint,          \
	s16: codec_field_sint,         \
	s32: codec_field_sint,         \
	s64: codec_field_sint,         \
	u8: codec_field_uint,          \
	u16: codec_field_uint,         \
	u32: codec_field_uint,         \
	u64: codec_field_uint,         \
	usize: codec_field_uint,       \
	float: codec_field_float,      \
	double: codec_field_float,     \
	char*: codec_field_string,     \
	string: codec_field_string,    \
	str: codec_field_string,       \
	default: codec_field_sub_codec,\
)*/
#define NIL_STRUCT_CODEC_FIELD(T, $field_type, $name) {#$name, NIL_STRUCT_CODEC_FIELD_TYPE($field_type), sizeof(struct T{}.$name), offsetof(struct T, $name), &TYPE_CODEC_NAME($field_type)}
#define NIL_STRUCT_CODEC_FIELD_TUPLE($tuple) NIL_STRUCT_CODEC_FIELD(NIL_IDENTITY $tuple)
// #define NIL_STRUCT_CODEC_ADD_TYPE(T, $tuple) (T, )
// #define NIL_STRUCT_CODEC_FIELD_TUPLE($tuple) NIL_IDENTITY $tuple

#define STRUCT_CODEC(T, ...) const codec TYPE_CODEC_NAME(T) = {                               \
	.type = codec_struct,                                                                      \
	.name = #T,                                                                                \
	.type_size = sizeof(struct T),                                                             \
	.mutable = true,                                                                           \
	.free = nullptr,                                                                           \
	.struct_data = {                                                                           \
		.field_count = NIL_COUNT_ARGS(__VA_ARGS__),                                             \
		.fields = (codec_field[]){NIL_WRAP_VA_ARGS(NIL_STRUCT_CODEC_FIELD_TUPLE, __VA_ARGS__)}, \
	},                                                                                         \
};

// Attempts to get an enum variant's name from a variant value based on the specified codec.
// Returns nullptr if either the codec is not a valid enum codec, or the variant_value is not in the codec.
const char* codec_enum_name_from_variant_value(const codec* codec, s64 variant_value);

// Prints an object out to the console with the specified codec.
void debug_with_codec(const void* obj, const codec* codec);