#pragma once

#include "nint.h"
#include "macro_utils.h"

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

	bool mutable; // TODO: Should we remove this? It currently isn't used.

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

void type_info_debug_print(const type_info* type, FILE* file);
bool type_info_contains_annotation(const type_info* type, const char* annotation);

// If applicable, calls the reflected free function of `type` on `data`.
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

// void type_register_trait(type_registry* reg, const type_info* trait_type, const void* trait_data);
// const void* type_registry_get_trait(const type_registry* reg, const type_info* trait_type);


void nil_register_default_types(type_registry* reg);

// For internal use only.
/*void nil_register_type_info__(const type_info* type);
const type_info* type_registry_get(usize index);
usize type_registry_len();
// TODO: Make inline?
usize type_registry_index(const type_info* type);*/
// const type_info* get_type_registry();

// typedef struct trait_registry trait_registry;

// trait_registry* trait_registry_new();

// -- TRAITS -- //

// #define NIL_TYPE_REGISTER_CONSTRUCTION_PRIORITY 101
// #define NIL_TRAIT_CONSTRUCTION_PRIORITY 150
// #define TYPE_TRAIT_CONSTRUCTOR __attribute__((constructor(NIL_TRAIT_CONSTRUCTION_PRIORITY)))

// Implementation. Do not use in header files.
// #define REFLECT_FREE_FN(T, FN) TYPE_TRAIT_CONSTRUCTOR static void NIL_GENERATED_COUNTER_NAME(reflect_destructor)() { type_info_register_free_fn(TYPE_INFO(T), (type_info_free_fn)FN); }

/*typedef struct sparse_trait_registry {

} sparse_trait_registry;*/

/*
#define DECLARE_SPARSE_TRAIT_REGISTRY(NAME, T) \
	const T* NAME##_get(const type_info* type);

#define DEFINE_SPARSE_TRAIT_REGISTRY(NAME, T) \
*/


#define NIL_TYPE_INFO_NAME(T) type_info_$_##T
#define NIL_TYPE_INFO_INDEX_NAME(T) type_info_index_$_##T
#define DECLARE_TYPE_INFO(T) extern const type_info NIL_TYPE_INFO_NAME(T);

#define DEFINE_TYPE_INFO(T, ...) const type_info NIL_TYPE_INFO_NAME(T) = { __VA_ARGS__ };

#define NIL_ELABORATED_TYPE_OPTION(_1, _2, NAME, ...) NAME

// Retrieves the `type_info` of a type. Namespaced types must be separated by a comma instead of a space due to macro limitations.
// For example: TYPE_INFO(struct,my_struct) or TYPE_INFO(my_struct_t) if it is behind a typedef.
#define TYPE_INFO(...) NIL_ELABORATED_TYPE_OPTION(__VA_ARGS__, NIL_TYPE_INFO_ELABORATED, NIL_TYPE_INFO_SIMPLE)(__VA_ARGS__)
#define NIL_TYPE_INFO_SIMPLE(T) ((void)sizeof(T),&NIL_TYPE_INFO_NAME(T))
#define NIL_TYPE_INFO_ELABORATED(TAG, T) ((void)sizeof(TAG T),&NIL_TYPE_INFO_NAME(T))

// Automatic reflection.
#define NIL_ANNOTATION_GENERATE_REFLECTION "auto_reflect"
#define NIL_ANNOTATION_REFLECT_IGNORE "reflect_ignore"
#define NIL_ANNOTATION_REFLECT_FREE_PREFIX "reflect_free"

#define ANNOTATE(STRING) __attribute__((annotate(STRING)))
#define REFLECT_IGNORE ANNOTATE(NIL_ANNOTATION_REFLECT_IGNORE)
#define REFLECT_FREE(FN) ANNOTATE(NIL_ANNOTATION_REFLECT_FREE_PREFIX "=" #FN)

#define NIL_REFLECTION_AUTO_REGISTER_MARKER(T) typedef T NIL_GENERATED_COUNTER_NAME(auto_register_marker) ANNOTATE(NIL_ANNOTATION_GENERATE_REFLECTION)

// Only use in header files or any file reflected in build options.
#define REFLECT_TYPE(...) NIL_ELABORATED_TYPE_OPTION(__VA_ARGS__, NIL_REFLECT_TYPE_ELABORATED, NIL_REFLECT_TYPE_SIMPLE)(__VA_ARGS__)
#define NIL_REFLECT_TYPE_SIMPLE(T) NIL_REFLECTION_AUTO_REGISTER_MARKER(T); DECLARE_TYPE_INFO(T)
#define NIL_REFLECT_TYPE_ELABORATED(TAG, T) NIL_REFLECTION_AUTO_REGISTER_MARKER(TAG T); DECLARE_TYPE_INFO(T)

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

DECLARE_TYPE_INFO(int)
DECLARE_TYPE_INFO(short)
DECLARE_TYPE_INFO(long)

DECLARE_TYPE_INFO(float)
DECLARE_TYPE_INFO(double)

// Attempts to get an enum variant's name from a variant value based on the specified codec.
// Returns nullptr if either the codec is not a valid enum codec, or the variant_value is not in the codec.
const char* reflect_enum_name_from_variant_value(const type_info* type, s64 variant_value);

// Prints an object out to the console with the specified codec.
void debug_reflected(const void* obj, const type_info* type);