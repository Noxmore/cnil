#pragma once

#include "nil/vec.h"
#include "nil/string.h"

typedef struct variant_builder variant_builder;
typedef struct field_builder field_builder;

typedef struct type_info_builder {
	enum type_info_kind kind;
	string name;
	vec(string) annotations;
	const char* free_fn;

	union {
		vec(field_builder) struct_fields;
		vec(variant_builder) enum_variants;
	};

	vec_anon(struct type_info_builder) sub_types;
} type_info_builder;
void type_info_builder_free(type_info_builder* builder);

typedef struct variant_builder {
	string name;
	vec(string) annotations;
} variant_builder;
void variant_builder_free(variant_builder* builder);

typedef struct field_builder {
	string name;
	vec(string) annotations;
	string field_type;
	type_info_builder* anon_type;
	bool is_pointer;
	bool is_const;
} field_builder;
void field_builder_free(field_builder* builder);

typedef struct reflect_ctx {
	// FILE* out;
	vec(type_info_builder) types;
	// const char* current_typename;
} reflect_ctx;
void reflect_ctx_free(reflect_ctx* ctx);