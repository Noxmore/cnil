#include "write.h"

#include "nil/hashtable.h"

#include <stdio.h>
#include <string.h>

#include "allocator.h"
#include "nil/hash.h"

static void indent(FILE* file, const u32 depth) {
	for (u32 i = 0; i < depth; i++)
		fputc('\t', file);
}

static void write_annotations(FILE* file, const char* field_prefix, const char* field_suffix, const string* annotations, const usize annotation_count) {
	if (annotation_count == 0)
		return;

	fputs(field_prefix, file);
	fprintf(file, ".annotations = (str[]){");
	for (usize i = 0; i < annotation_count; i++) {
		fprintf(file, "s(");
		fprintf(file, "\"%s\"", annotations[i].data);
		fputc(')', file);
		if (i != annotation_count-1)
			fprintf(file, ", ");
	}
	fprintf(file, "},");
	fputs(field_suffix, file);

	fputs(field_prefix, file);
	fprintf(file, ".annotation_count = %lu,", annotation_count);
	fputs(field_suffix, file);
}



static const char* type_info_kind_as_string(const enum type_info_kind kind) {
	switch (kind) {
		case type_info_struct: return "type_info_struct";
		case type_info_enum: return "type_info_enum";
		case type_info_union: return "type_info_union";
		case type_info_opaque: return "type_info_opaque";
		default: return nullptr;
	}
}

static void write_parsed_type(FILE* file, const type_info_builder* type, const u32 depth) {
	// We need to pass a suffix to write_annotations anyway, so we calculate this up front.
	const u32 body_indent_chars = depth + 1;
	char body_indent[body_indent_chars + 1];
	body_indent[body_indent_chars] = 0;
	memset(body_indent, '\t', body_indent_chars);

	fputs(body_indent, file); fprintf(file, ".kind = %s,\n", type_info_kind_as_string(type->kind));

	// fputs(body_indent, file); fprintf(file, ".name = s(\"%s\"),\n", type->name.data);
	// Don't do this for anonymous types.
	// const char* type_prefix = get_type_prefix(type);
	if (type->name.data != nullptr) {
		fputs(body_indent, file); fprintf(file, ".size = sizeof(%s),\n", type->type_referral.data);
		fputs(body_indent, file); fprintf(file, ".align = alignof(%s),\n", type->type_referral.data);
	}

	write_annotations(file, body_indent, "\n", type->annotations.data, type->annotations.len);

	/*fputs(body_indent, file);
	if (type->free_fn.data == nullptr) {
		fprintf(file, ".free = nullptr,\n");
	} else {
		fprintf(file, ".free = (nil_free_fn)%s,\n", type->free_fn.data);
	}*/

	if (type->kind == type_info_struct || type->kind == type_info_union) {
		fputs(body_indent, file); fprintf(file, ".struct_data.field_count = %lu,\n", type->struct_fields.len);
		fputs(body_indent, file);
		if (type->struct_fields.len == 0) {
			fprintf(file, ".struct_data.fields = nullptr,\n");
		} else {
			fprintf(file, ".struct_data.fields = (type_info_field[]){\n");
			for (usize field_idx = 0; field_idx < type->struct_fields.len; field_idx++) {
				const field_builder* field = &type->struct_fields.data[field_idx];

				indent(file, depth+2);
				fprintf(file, "{ .name = s(\"%s\"), ", field->name.data);
				write_annotations(file, "", " ", field->annotations.data, field->annotations.len);

				fprintf(file, ".struct_offset = __builtin_offsetof(%s, %s), ", type->type_referral.data, field->name.data);
				fprintf(file, ".size = sizeof((%s){}.%s), ", type->type_referral.data, field->name.data);

				if (field->const_array_layers.len > 0) {
					fprintf(file, ".const_array_layers = (usize[]){ ");
					for (usize i = 0; i < field->const_array_layers.len; i++) {
						fprintf(file, "%lu", field->const_array_layers.data[i]);

						if (i != field->const_array_layers.len-1)
							fprintf(file, ", ");
					}
					fprintf(file, " }, .const_array_layer_count = %lu, ", field->const_array_layers.len);
				}

				if (field->pointer_layers > 0)
					fprintf(file, ".pointer_layers = %u, ", field->pointer_layers);
				if (field->is_const)
					fprintf(file, ".is_const = true, ");

				fprintf(file, ".type = ");
				if (field->anon_type != nullptr) {
					fprintf(file, "&(type_info){\n");
					indent(file, depth + 3);
					fprintf(file, ".name = s(\"%s\"),\n", field->anon_type->name.data);
					indent(file, depth + 3);
					fprintf(file, ".mutable = true,\n");
					write_parsed_type(file, field->anon_type, depth + 2);
					indent(file, depth+2);
					fprintf(file, "} ");
				} else {
					fprintf(file, "&NIL_TYPE_INFO_NAME(%s) ", field->field_type.data);
				}

				fprintf(file, "},\n");
			}
			fputs(body_indent, file); fprintf(file, "},\n");
		}
	} else if (type->kind == type_info_enum) {
		fputs(body_indent, file); fprintf(file, ".enum_data.variant_count = %lu,\n", type->enum_variants.len);
		fputs(body_indent, file);
		if (type->enum_variants.len == 0) {
			fprintf(file, ".enum_data.variants = nullptr,\n");
		} else {
			fprintf(file, ".enum_data.variants = (type_info_variant[]){\n");
			for (usize i = 0; i < type->enum_variants.len; i++) {
				const variant_builder* variant = &type->enum_variants.data[i];

				indent(file, depth+2);
				fprintf(file, "{ .name = s(\"%s\"), ", variant->name.data);
				fprintf(file, ".value = %s, ", variant->name.data);
				write_annotations(file, "", " ", variant->annotations.data, variant->annotations.len);
				fprintf(file, "},\n");
			}
			fputs(body_indent, file); fprintf(file, "},\n");
		}
	}
}

/*typedef struct write_ctx {
	generic_hash_set
} write_ctx;*/

static void write_type_and_subtype_definitions(FILE* file, const type_info_builder* type, hashset(u64)* written_type_names) {
	for (usize i = 0; i < type->sub_types.len; i++) {
		u64 hash = nil_hash(type->name.data, type->name.len);
		if (hashset_contains(written_type_names, &hash))
			continue;

		fprintf(file, "static ");
		write_type_and_subtype_definitions(file, &type->sub_types.data[i], written_type_names);
		hashset_insert(written_type_names, alloc, hash);
	}

	fprintf(file, "DEFINE_TYPE_INFO(%s, \n", type->name.data);
	write_parsed_type(file, type, 0);
	fprintf(file, ")\n");
}

void write_parsed_types(FILE* file, const reflect_ctx* ctx, const char* header_file) {
	if (ctx->types.len == 0) {
		fprintf(file, "// Stub - No types reflected.");
		return;
	}

	fprintf(file, "#include \"nil/reflect.h\"\n");
	fprintf(file, "#include \"%s\"\n\n", header_file);

	hashset(u64) written_type_names = {};
	written_type_names.policy = &nil_passthrough_hashtable_policy;

	for (usize type_idx = 0; type_idx < ctx->types.len; type_idx++) {
		const type_info_builder* type = &ctx->types.data[type_idx];

		write_type_and_subtype_definitions(file, type, &written_type_names);
	}
}