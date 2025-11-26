#include "write.h"

#include <stdio.h>

static void indent(FILE* file, const u32 depth) {
	for (u32 i = 0; i < depth; i++)
		fputc('\t', file);
}

static void write_annotations(FILE* file, const char* prefix, const char* suffix, const string* annotations, const usize annotation_count) {
	fputs(prefix, file);
	if (annotation_count == 0) {
		fprintf(file, ".annotations = nullptr,");
		fputs(suffix, file);
	} else {
		fprintf(file, ".annotations = (const char*[]){");
		for (usize i = 0; i < annotation_count; i++) {
			fprintf(file, "\"%s\"", annotations[i].data);
			if (i != annotation_count-1)
				fprintf(file, ", ");
		}
		fprintf(file, "},");
		fputs(suffix, file);
	}
	fputs(prefix, file); fprintf(file, ".annotation_count = %lu,", annotation_count);
	fputs(suffix, file);
}

static const char* get_type_prefix(const type_info_builder* type) {
	if (type->no_namespace)
		return "";

	switch (type->kind) {
		case type_info_struct: return "struct ";
		case type_info_enum: return "enum ";
		case type_info_union: return "union ";
		default: return "";
	}
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

	fputs(body_indent, file); fprintf(file, ".name = \"%s\",\n", type->name.data);
	const char* type_prefix = get_type_prefix(type);
	fputs(body_indent, file); fprintf(file, ".size = sizeof(%s%s),\n", type_prefix, type->name.data);
	fputs(body_indent, file); fprintf(file, ".align = alignof(%s%s),\n", type_prefix, type->name.data);

	write_annotations(file, body_indent, "\n", type->annotations.data, type->annotations.len);

	// fputs(body_indent, file); fprintf(file, ".free = %s,\n", type->free_fn == nullptr ? "nullptr" : type->free_fn);

	if (type->kind == type_info_struct || type->kind == type_info_union) {
		fputs(body_indent, file); fprintf(file, ".struct_data.field_count = %lu,\n", type->struct_fields.len);
		fputs(body_indent, file);
		if (type->struct_fields.len == 0) {
			fprintf(file, ".struct_data.fields = nullptr,\n");
		} else {
			fprintf(file, ".struct_data.fields = (type_info_field[]){\n");
			for (usize i = 0; i < type->struct_fields.len; i++) {
				const field_builder* field = &type->struct_fields.data[i];

				indent(file, depth+2);
				fprintf(file, "{ .name = \"%s\", ", field->name.data);
				write_annotations(file, "", " ", field->annotations.data, field->annotations.len);
				fprintf(file, ".field_type = ");
				if (field->anon_type != nullptr) {
					fprintf(file, "&(type_info){\n");
					write_parsed_type(file, field->anon_type, depth + 1);
					fprintf(file, "}, ");
				} else {
					fprintf(file, "&NIL_TYPE_INFO_NAME(%s), ", field->field_type.data);
				}
				fprintf(file, ".struct_offset = __builtin_offsetof(%s%s, %s), ", type_prefix, type->name.data, field->name.data);
				fprintf(file, ".is_pointer = %s, ", field->is_pointer ? "true" : "false");
				fprintf(file, ".is_const = %s ", field->is_const ? "true" : "false");
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
				fprintf(file, "{ .name = \"%s\", ", variant->name.data);
				fprintf(file, ".value = %s, ", variant->name.data);
				write_annotations(file, "", " ", variant->annotations.data, variant->annotations.len);
				fprintf(file, "},\n");
			}
			fputs(body_indent, file); fprintf(file, "},\n");
		}
	}
}

static void write_type_and_subtype_definitions(FILE* file, const type_info_builder* type) {
	for (usize i = 0; i < type->sub_types.len; i++)
		write_type_and_subtype_definitions(file, &type->sub_types.data[i]);

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

	for (usize type_idx = 0; type_idx < ctx->types.len; type_idx++) {
		const type_info_builder* type = &ctx->types.data[type_idx];

		write_type_and_subtype_definitions(file, type);
	}
}