#include "data.h"

#include <stdlib.h>

void variant_builder_free(variant_builder* builder) {
	string_free(&builder->name);
	vec_free_with(&builder->annotations, string_free);
}

void field_builder_free(field_builder* builder) {
	string_free(&builder->name);
	vec_free_with(&builder->annotations, string_free);
	string_free(&builder->field_type);
	if (builder->anon_type) {
		type_info_builder_free(builder->anon_type);
		free(builder->anon_type);
	}
}

void type_info_builder_free(type_info_builder* builder) {
	string_free(&builder->name);
	vec_free_with(&builder->annotations, string_free);
	if (builder->kind == type_info_struct) vec_free_with(&builder->struct_fields, field_builder_free);
	else if (builder->kind == type_info_enum) vec_free_with(&builder->enum_variants, variant_builder_free);
	vec_free_with(&builder->sub_types, type_info_builder_free);
}

void reflect_ctx_free(reflect_ctx* ctx) {
	vec_free_with(&ctx->types, type_info_builder_free);
}