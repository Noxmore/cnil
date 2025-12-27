#include "data.h"

#include <stdlib.h>

#include "nil/hash.h"

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
	string_free(&builder->type_referral);
	vec_free_with(&builder->annotations, string_free);
	// string_free(&builder->free_fn);
	if (builder->kind == type_info_struct) vec_free_with(&builder->struct_fields, field_builder_free);
	else if (builder->kind == type_info_enum) vec_free_with(&builder->enum_variants, variant_builder_free);
	vec_free_with(&builder->sub_types, type_info_builder_free);
}
void reflect_ctx_free(reflect_ctx* ctx) {
	vec_free_with(&ctx->types, type_info_builder_free);
}

/*u64 type_hash(const type_info_builder* type) {
	u64 seed = nil_hash_single(type->kind);
	seed = nil_hash_with_seed(seed, type->name.data, type->name.len);
	seed = nil_hash_single_with_seed(seed, type->no_namespace);

	seed = nil_hash_single_with_seed(seed, type->annotations.len);
	for (usize i = 0; i < type->annotations.len; i++)
		seed = nil_hash_with_seed(seed, type->annotations.data[i].data, type->annotations.data[i].len);

	seed = nil_hash_with_seed(seed, type->free_fn.data, type->free_fn.len);
}*/