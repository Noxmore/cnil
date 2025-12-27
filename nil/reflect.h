#pragma once

#include "nint.h"
#ifdef NIL_INCLUDE_ANNOTATIONS
	#include "macro_utils.h"
#endif
#include "string.h"

#include <bits/types/FILE.h>

void nil_integer_to_bytes(s64 value, u8* out, usize n);
s64 nil_bytes_to_integer(const u8* in, usize n);

// Generic free function/destructor.
// I can't think of a better place to put this, so it's going into reflect.h!
typedef void (*nil_free_fn)(void*);

typedef struct type_info type_info;

typedef const char* const* type_info_annotations;

typedef struct type_info_field {
	// Name of the field. Null terminated.
	str name;

	type_info_annotations annotations;
	usize annotation_count;

	const type_info* field_type;

	u32 struct_offset;

	bool is_pointer;
	bool is_const;
} type_info_field;

// Reflected enum variant
typedef struct type_info_variant {
	str name;
	s64 value;
	type_info_annotations annotations;
	usize annotation_count;
} type_info_variant;

typedef enum type_info_generic {
	type_info_generic_type,
	type_info_generic_data_types,
} type_info_generic;

// Describes an in-memory C data structure, providing reflection. This is particularly useful for editors, serialization (for example, using NON), and other type-based introspection.
typedef struct type_info {
	enum type_info_kind {
		type_info_struct,
		type_info_enum,
		type_info_union,
		// Things like integers and strings that are the primitive building blocks that other types contain.
		type_info_opaque,
		type_info_kinds,
	} kind;

	bool mutable; // TODO: Should we remove this? It currently isn't used.

	// Optional null-terminated type name. Can be null.
	str name;
	usize size;
	usize align;

	type_info_annotations annotations;
	usize annotation_count;

	/*// Custom free function. Can be nullptr.
	nil_free_fn free;

	// Conversion functions. Mainly used for opaque/primitive types.
	type_info_conversions conversions;*/

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
			// The kind of data the opaque type should be treated as. For example, if it's a string, code might try to run `to_string` and `from_string` conversions to interact with it.
			enum type_info_opaque_kind {
				type_info_opaque_other = 0,
				type_info_opaque_void,
				type_info_opaque_sint,
				type_info_opaque_uint,
				type_info_opaque_real,
				type_info_opaque_string,
				type_info_opaque_kinds,
			} kind;
		} opaque_data;

		struct {
			const type_info* specialization;

		} specialization_data;
	};
} type_info;

void type_info_debug_print(const type_info* type, FILE* file);
bool type_info_contains_annotation(const type_info* type, const char* annotation);

// If applicable, calls the reflected free function of `type` on `data`. Otherwise, recursively searches for the free function in fields.
void free_reflected(const type_info* type, void* data);

typedef struct type_registry type_registry;

type_registry* type_registry_new();
// Create a new type registry with an initial allocated size.
type_registry* type_registry_new_with(usize reserve);

void type_register(type_registry* reg, const type_info* type);
usize type_registry_size(const type_registry* reg);
// If this returns `nullptr`, `type` is not in this registry.
const usize* type_registry_index(type_registry* reg, const type_info* type);
bool type_registry_contains(const type_registry* reg, const type_info* type);
const type_info* type_registry_get(const type_registry* reg, usize index);

void type_registry_free(type_registry* reg);

void nil_register_default_types(type_registry* reg);



#define NIL_TYPE_INFO_NAME(T) type_info_$_##T
#define NIL_TYPE_INFO_INDEX_NAME(T) type_info_index_$_##T
#define DECLARE_TYPE_INFO(T) extern const type_info NIL_TYPE_INFO_NAME(T);

#define DEFINE_TYPE_INFO(T, ...) const type_info NIL_TYPE_INFO_NAME(T) = { .mutable = true, .name = s(#T), __VA_ARGS__ };
// `DEFINE_TYPE_INFO` can't automatically specify size and alignment because of type namespaces. This assumes T is in the global namespace.
#define DEFINE_TYPEDEF_INFO(T, ...) DEFINE_TYPE_INFO(T, .size = sizeof(T), .align = alignof(T), __VA_ARGS__)

#define NIL_ELABORATED_TYPE_OPTION(_1, _2, NAME, ...) NAME

// Retrieves the `type_info` of a type. Namespaced types must be separated by a comma instead of a space due to macro limitations.
// For example: TYPE_INFO(struct,my_struct) or just TYPE_INFO(my_struct) if it is behind a typedef.
#define TYPE_INFO(...) NIL_ELABORATED_TYPE_OPTION(__VA_ARGS__, NIL_TYPE_INFO_ELABORATED, NIL_TYPE_INFO_SIMPLE)(__VA_ARGS__)
#define NIL_TYPE_INFO_SIMPLE(T) ((void)sizeof(T),&NIL_TYPE_INFO_NAME(T))
#define NIL_TYPE_INFO_ELABORATED(TAG, T) ((void)sizeof(TAG T),&NIL_TYPE_INFO_NAME(T))

// Automatic reflection.
#define NIL_ANNOTATION_GENERATE_REFLECTION "auto_reflect"
#define NIL_ANNOTATION_REFLECT_IGNORE "reflect_ignore"
// #define NIL_ANNOTATION_REFLECT_FREE_PREFIX "reflect_free"
#define NIL_ANNOTATION_DOC_PREFIX "doc"
// #define NIL_ANNOTATION_ARRAY_PREFIX "reflect_array"

#ifdef NIL_INCLUDE_ANNOTATIONS
	#define ANNOTATE(STRING) __attribute__((annotate(STRING)))
#else
	#define ANNOTATE(STRING)
#endif
#define REFLECT_IGNORE ANNOTATE(NIL_ANNOTATION_REFLECT_IGNORE)
// #define REFLECT_FREE(FN) ANNOTATE(NIL_ANNOTATION_REFLECT_FREE_PREFIX "=" #FN)
#define REFLECT_DOC(DOC) ANNOTATE(NIL_ANNOTATION_DOC_PREFIX "=" DOC)
// #define REFLECT_ARRAY()

#ifdef NIL_INCLUDE_ANNOTATIONS
	#define NIL_REFLECTION_AUTO_REGISTER_MARKER(T) typedef T NIL_GENERATED_COUNTER_NAME(auto_register_marker) ANNOTATE(NIL_ANNOTATION_GENERATE_REFLECTION);
#else
	#define NIL_REFLECTION_AUTO_REGISTER_MARKER(T)
#endif

// Only use in header files or any file reflected in build options.
#define REFLECT_TYPE(...) NIL_ELABORATED_TYPE_OPTION(__VA_ARGS__, NIL_REFLECT_TYPE_ELABORATED, NIL_REFLECT_TYPE_SIMPLE)(__VA_ARGS__)
#define NIL_REFLECT_TYPE_SIMPLE(T) NIL_REFLECTION_AUTO_REGISTER_MARKER(T) DECLARE_TYPE_INFO(T)
#define NIL_REFLECT_TYPE_ELABORATED(TAG, T) NIL_REFLECTION_AUTO_REGISTER_MARKER(TAG T) DECLARE_TYPE_INFO(T)

DECLARE_TYPE_INFO(void)

DECLARE_TYPE_INFO(u8)
DECLARE_TYPE_INFO(u16)
DECLARE_TYPE_INFO(u32)
DECLARE_TYPE_INFO(u64)
DECLARE_TYPE_INFO(u128)
DECLARE_TYPE_INFO(usize)

DECLARE_TYPE_INFO(uint)
DECLARE_TYPE_INFO(ushort)
DECLARE_TYPE_INFO(ulong)

DECLARE_TYPE_INFO(size_t)

DECLARE_TYPE_INFO(s8)
DECLARE_TYPE_INFO(s16)
DECLARE_TYPE_INFO(s32)
DECLARE_TYPE_INFO(s64)
DECLARE_TYPE_INFO(s128)

DECLARE_TYPE_INFO(char)
DECLARE_TYPE_INFO(short)
DECLARE_TYPE_INFO(int)
DECLARE_TYPE_INFO(long)

DECLARE_TYPE_INFO(float)
DECLARE_TYPE_INFO(double)

DECLARE_TYPE_INFO(string)
DECLARE_TYPE_INFO(str)

// Attempts to get an enum variant's name from a variant value based on the specified codec. The resulting str will be null-terminated.
// Returns a null str if either the codec is not a valid enum codec, or the variant_value is not in the codec.
str reflect_enum_name_from_variant_value(const type_info* type, s64 variant_value);
// Returns whether the enum represented by `type` contains a variant with the specified value.
// If the type does not represent an enum, returns `false`.
bool reflected_enum_contains_variant_value(const type_info* type, s64 variant_value);

// Internal tool for converting an arbitrarily-sized integer from reflected types. Often used with `reflect_enum_name_from_variant_value`.
// s64 nil_get_any_sized_int(const void* data, u8 type_size);

// Prints an object out to the console dynamically using its reflected `type_info`.
void debug_reflected(const void* obj, const type_info* type);
