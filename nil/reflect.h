#pragma once

#include "nint.h"

#include <bits/types/FILE.h>

typedef struct type_info type_info;

typedef const char* const* type_info_annotations;

typedef struct type_info_field {
	const char* name;

	type_info_annotations annotations;
	usize annotation_count;

	const type_info* field_type;

	u32 struct_offset;

	bool is_pointer;
	bool is_const;
} type_info_field;

// Reflected enum variant
typedef struct type_info_variant {
	const char* name;
	s64 value;
	type_info_annotations annotations;
	usize annotation_count;
} type_info_variant;

typedef void (*type_info_free_fn)(void*);

// Describes an in-memory data structure, providing a form of reflection. This is particularly useful for serialization.
typedef struct type_info {
	enum type_info_kind : u8 {
		type_info_struct,
		type_info_enum,
		type_info_union,
		type_info_opaque,
		type_info_kinds,
	} kind;

	bool mutable;

	// Optional type name. Can be nullptr.
	const char* name;
	usize size;
	usize align;

	type_info_annotations annotations;
	usize annotation_count;

	// Custom free function. Can be nullptr.
	type_info_free_fn free;

	union {
		struct {
			usize field_count;
			type_info_field* fields;
		} struct_data;

		struct {
			usize variant_count;
			type_info_variant* variants;
		} enum_data;

		// TODO: Special case for union data/tagged unions?

		struct {
			// Used for coloring and the like.
			enum type_info_opaque_kind : u8 {
				type_info_opaque_other,
				type_info_opaque_sint,
				type_info_opaque_uint,
				type_info_opaque_real,
				type_info_opaque_string,
				type_info_opaque_kinds,
			} kind;
			void (*to_string)(FILE*, const void*);
			void (*from_string)(const char* src, char** end_ptr, void* dst);
		} opaque_data;
	};
} type_info;

#define NIL_TYPE_INFO_NAME(T) type_info_$_##T
#define EXPORT_TYPE(T) extern const type_info NIL_TYPE_INFO_NAME(T);
#define DEFINE_TYPE_INFO(T) const type_info NIL_TYPE_INFO_NAME(T) =

#define type_info_of(T) (&NIL_TYPE_INFO_NAME(T))

// Automatic reflection.
#define NIL_ANNOTATION_REFLECT "reflect"
#define NIL_ANNOTATION_REFLECT_IGNORE "reflect_ignore"

#define ANNOTATE(STRING) __attribute__((annotate(STRING)))
#define reflected ANNOTATE(NIL_ANNOTATION_REFLECT)
#define reflect_ignore ANNOTATE(NIL_ANNOTATION_REFLECT_IGNORE)

EXPORT_TYPE(u8)
EXPORT_TYPE(u16)
EXPORT_TYPE(u32)
EXPORT_TYPE(u64)
EXPORT_TYPE(usize)

EXPORT_TYPE(uint)
EXPORT_TYPE(ushort)
EXPORT_TYPE(ulong)

EXPORT_TYPE(size_t)

EXPORT_TYPE(s8)
EXPORT_TYPE(s16)
EXPORT_TYPE(s32)
EXPORT_TYPE(s64)

EXPORT_TYPE(int)
EXPORT_TYPE(short)
EXPORT_TYPE(long)

EXPORT_TYPE(float)
EXPORT_TYPE(double)

/*#define ENUM_CODEC(T, ...) DEFINE_CODEC(T) {                         \
	.type = codec_enum,                                                                \
	.name = #T,                                                                        \
	.size = sizeof(enum T),                                                       \
	.align = alignof(enum T),                                                     \
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
)#1#
#define NIL_STRUCT_CODEC_FIELD(T, $field_type, $name) {#$name, NIL_STRUCT_CODEC_FIELD_TYPE($field_type), sizeof(struct T{}.$name), offsetof(struct T, $name), &TYPE_CODEC_NAME($field_type)}
#define NIL_STRUCT_CODEC_FIELD_TUPLE($tuple) NIL_STRUCT_CODEC_FIELD(NIL_IDENTITY $tuple)
// #define NIL_STRUCT_CODEC_ADD_TYPE(T, $tuple) (T, )
// #define NIL_STRUCT_CODEC_FIELD_TUPLE($tuple) NIL_IDENTITY $tuple

#define STRUCT_CODEC(T, ...) const codec TYPE_CODEC_NAME(T) = {                               \
	.type = codec_struct,                                                                      \
	.name = #T,                                                                                \
	.size = sizeof(struct T),                                                             \
	.align = alignof(struct T),                                                           \
	.mutable = true,                                                                           \
	.free = nullptr,                                                                           \
	.struct_data = {                                                                           \
		.field_count = NIL_COUNT_ARGS(__VA_ARGS__),                                             \
		.fields = (codec_field[]){NIL_WRAP_VA_ARGS(NIL_STRUCT_CODEC_FIELD_TUPLE, __VA_ARGS__)}, \
	},                                                                                         \
};*/

// Attempts to get an enum variant's name from a variant value based on the specified codec.
// Returns nullptr if either the codec is not a valid enum codec, or the variant_value is not in the codec.
const char* reflect_enum_name_from_variant_value(const type_info* type, s64 variant_value);

// Prints an object out to the console with the specified codec.
void debug_reflected(const void* obj, const type_info* type);