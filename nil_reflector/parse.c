#include "parse.h"

#include <clang-c/Index.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "nil/ansi_colors.h"
#include "nil/panic.h"
#include "nil/string.h"

/*static void debug_display_name(CXCursor cursor) {
	const CXString debug = clang_getCursorDisplayName(cursor);
	printf("DEBUG: %s\n", clang_getCString(debug));
	clang_disposeString(debug);
}*/

static bool is_display_name_eq(CXCursor cursor, const char* s) {
	const CXString display_name = clang_getCursorDisplayName(cursor);
	const bool is_eq = strcmp(clang_getCString(display_name), s) == 0;
	clang_disposeString(display_name);
	return is_eq;
}

static enum type_info_kind cursor_kind_to_type_info_kind(const enum CXCursorKind kind) {
	switch (kind) {
		case CXCursor_StructDecl: return type_info_struct;
		case CXCursor_UnionDecl: return type_info_union;
		case CXCursor_EnumDecl: return type_info_enum;
		default: return type_info_opaque; // Shouldn't happen.
	}
}

/*static bool parse_ctx_contains_type(const reflect_ctx* ctx, CXCursor cursor) {
	for (usize i = 0; i < ctx->types.len; i++) {
		const type_info_builder* type = &ctx->types.data[i];
		if (is_display_name_eq(cursor, type->name.data) && cursor_kind_to_type_info_kind(clang_getCursorKind(cursor)) == type->kind)
			return true;
	}

	return false;
}*/

// Takes ownership of `s`, disposing it in the process.
static string convert_str(CXString s) {
	const string out = string_new(clang_getCString(s));
	clang_disposeString(s);
	return out;
}

static string cursor_display_name(const CXCursor cursor) {
	return convert_str(clang_getCursorDisplayName(cursor));
}

static string cursor_spelling(const CXCursor cursor) {
	return convert_str(clang_getCursorSpelling(cursor));
}

static enum CXChildVisitResult push_annotations(CXCursor cursor, CXCursor parent, CXClientData client_data) {
	if (clang_getCursorKind(cursor) == CXCursor_AnnotateAttr) {
		vec_push((vec(string)*)client_data, cursor_display_name(cursor));
	}

	return CXChildVisit_Continue;
}

static enum CXChildVisitResult should_ignore_field_visitor(CXCursor cursor, CXCursor parent, CXClientData client_data) {
	if (clang_getCursorKind(cursor) == CXCursor_AnnotateAttr && is_display_name_eq(cursor, NIL_ANNOTATION_REFLECT_IGNORE)) {
		*(bool*)client_data = true;
		return CXChildVisit_Break;
	}

	return CXChildVisit_Continue;
}

static bool should_ignore_field(CXCursor cursor) {
	bool ignore = false;
	clang_visitChildren(cursor, should_ignore_field_visitor, &ignore);
	return ignore;
}

static enum CXChildVisitResult reflect_type(CXCursor cursor, CXCursor parent, CXClientData client_data) {
	type_info_builder* type = client_data;

	const enum CXCursorKind kind = clang_getCursorKind(cursor);

	if (kind == CXCursor_AnnotateAttr) {
		// Attempt to parse special
		const CXString display_name = clang_getCursorDisplayName(cursor);
		if (strcmp(clang_getCString(display_name), NIL_ANNOTATION_GENERATE_REFLECTION) == 0)
			return CXChildVisit_Continue;

		nil_string_splitter splitter = nil_split_cstr(clang_getCString(display_name), "=");
		str split;
		if (
			nil_split_next(&splitter, &split) &&
			str_eq_cstr(split, NIL_ANNOTATION_REFLECT_FREE_PREFIX) &&
			nil_split_next(&splitter, &split)
		) {
			type->free_fn = str_allocate(split);
			return CXChildVisit_Continue;
		}

		clang_disposeString(display_name);

		vec_push(&type->annotations, cursor_display_name(cursor));
	} else if (kind == CXCursor_FieldDecl) {
		if (should_ignore_field(cursor))
			return CXChildVisit_Continue;

		CXType field_type = clang_getCursorType(cursor);

		field_builder field = {0};
		field.name = cursor_display_name(cursor);
		clang_visitChildren(cursor, push_annotations, &field.annotations);

		field.is_pointer = field_type.kind == CXType_Pointer;
		field.is_const = clang_isConstQualifiedType(field_type);

		if (field_type.kind == CXType_Pointer)
			field_type = clang_getPointeeType(field_type);
		if (field_type.kind == CXType_Elaborated)
			field_type = clang_Type_getNamedType(field_type);
		if (field_type.kind == CXType_Invalid) {
			fprintf(stderr, "Invalid type from field `%s`", field.name.data);
			return CXChildVisit_Continue;
		}

		// I don't fully know what these functions do.
		if (clang_Cursor_isAnonymousRecordDecl(cursor) || clang_Cursor_isAnonymous(cursor)) {
			type_info_builder* sub_type = malloc(sizeof(type_info_builder));
			memset(sub_type, 0, sizeof(type_info_builder));

			const CXCursor field_type_cursor = clang_getTypeDeclaration(field_type);

			sub_type->kind = clang_getCursorKind(field_type_cursor) == CXCursor_EnumDecl ? type_info_enum : type_info_struct;
			clang_visitChildren(cursor, reflect_type, sub_type);

			field.anon_type = sub_type;
		} else {
			if (field_type.kind == CXType_Record || field_type.kind == CXType_Enum) {
				field.field_type = cursor_spelling(clang_getTypeDeclaration(field_type));
			} else {
				field.field_type = convert_str(clang_getTypeSpelling(field_type));
			}
		}

		vec_push(&type->struct_fields, field);
	} else if (kind == CXCursor_EnumConstantDecl) {
		if (should_ignore_field(cursor))
			return CXChildVisit_Continue;

		variant_builder variant = {0};
		variant.name = cursor_display_name(cursor);
		clang_visitChildren(cursor, push_annotations, &variant.annotations);
		vec_push(&type->enum_variants, variant);
	} else if (kind == CXCursor_StructDecl || kind == CXCursor_EnumDecl || kind == CXCursor_UnionDecl) {
		// I don't fully know what these functions do.
		if (clang_Cursor_isAnonymousRecordDecl(cursor) || clang_Cursor_isAnonymous(cursor)) {
			return CXChildVisit_Continue;
		}

		type_info_builder sub_type = {0};
		sub_type.kind = kind == CXCursor_EnumDecl ? type_info_enum : type_info_struct;
		sub_type.name = cursor_display_name(cursor);

		clang_visitChildren(cursor, reflect_type, &sub_type);

		vec_push(&type->sub_types, sub_type);
	}

	return CXChildVisit_Continue;
}

static enum CXChildVisitResult is_marked_for_reflection(CXCursor cursor, CXCursor parent, CXClientData client_data) {
	if (clang_getCursorKind(cursor) == CXCursor_AnnotateAttr && is_display_name_eq(cursor, NIL_ANNOTATION_GENERATE_REFLECTION)) {
		*(bool*)client_data = true;
		return CXChildVisit_Break;
	}

	return CXChildVisit_Continue;
}

static bool is_in_header(CXCursor cursor, const reflect_ctx* ctx) {
	CXFile file;
	unsigned line, column, offset;
	clang_getExpansionLocation(clang_getCursorLocation(cursor), &file, &line, &column, &offset);

	const CXString filename = clang_getFileName(file);
	if (clang_getCString(filename) == nullptr) {
		return true;
	}
	char canonical_cursor_file_path[PATH_MAX];
	realpath(clang_getCString(filename), canonical_cursor_file_path);
	clang_disposeString(filename);

	return strcmp(canonical_cursor_file_path, ctx->canonical_file_path) != 0;
}

static enum CXChildVisitResult search_for_reflected_types(CXCursor cursor, CXCursor parent, CXClientData client_data) {
	reflect_ctx* ctx = client_data;

	// if (clang_Location_isFromMainFile(clang_getCursorLocation(cursor)) != 0)
	// 	return CXChildVisit_Continue;
	if (is_in_header(cursor, ctx))
		return CXChildVisit_Continue;
	// clang_getCursor
	// Allocate a CXString representing the name of the current cursor


	/*if (kind == CXCursor_StructDecl || kind == CXCursor_EnumDecl || kind == CXCursor_UnionDecl) {
		bool should_be_reflected = false;
		clang_visitChildren(cursor, is_marked_for_reflection, &should_be_reflected);

		reflect_ctx* ctx = client_data;

		if (should_be_reflected && !parse_ctx_contains_type(ctx, cursor)) {
			// debug_display_name(parent);
			// debug_display_name(current_cursor);
			// const CXString typename = clang_getCursorDisplayName(current_cursor);

			type_info_builder builder = {0};
			builder.kind = cursor_kind_to_type_info_kind(kind);
			builder.name = cursor_display_name(cursor);
			clang_visitChildren(cursor, reflect_type, &builder);

			vec_push(&ctx->types, builder);
		}

		return CXChildVisit_Continue;
	}*/
	if (clang_getCursorKind(cursor) == CXCursor_TypedefDecl) {
		bool should_be_reflected = false;
		clang_visitChildren(cursor, is_marked_for_reflection, &should_be_reflected);

		if (should_be_reflected) {
			// debug_display_name(current_cursor);
			// CXCursor type_cursor = clang_getTypeDeclaration(clang_getTypedefDeclUnderlyingType(cursor));
			CXCursor type_cursor = clang_getTypeDeclaration(clang_getCanonicalType(clang_getTypedefDeclUnderlyingType(cursor)));
			// Trace back through all layers of typedefs.
			// while (clang_getCursorKind(type_cursor) == CXCursor_TypedefDecl)
			// 	type_cursor = clang_getTypeDeclaration(clang_getTypedefDeclUnderlyingType(type_cursor));
			const enum CXCursorKind type_cursor_kind = clang_getCursorKind(type_cursor);

			// Make sure we can actually reflect the type this typedef points to.
			if (type_cursor_kind != CXCursor_StructDecl && type_cursor_kind != CXCursor_EnumDecl && type_cursor_kind != CXCursor_UnionDecl)
				return CXChildVisit_Continue;

			type_info_builder builder = {0};
			builder.kind = cursor_kind_to_type_info_kind(type_cursor_kind);
			builder.name = cursor_display_name(type_cursor);
			// Backup name, just in case versions/compilers differ on things like anonymous structs.
			if (builder.name.data == nullptr)
				builder.name = convert_str(clang_getTypeSpelling(clang_getTypedefDeclUnderlyingType(cursor)));

			const CXString typedef_spelling = clang_getTypeSpelling(clang_getTypedefDeclUnderlyingType(cursor));
			builder.no_namespace = strcmp(clang_getCString(typedef_spelling), builder.name.data) == 0;
			clang_disposeString(typedef_spelling);

			clang_visitChildren(type_cursor, reflect_type, &builder);

			vec_push(&ctx->types, builder);
		}

		return CXChildVisit_Continue;
	}

	return CXChildVisit_Recurse;
}

// Yes. This is stupid, and is on necessary on _some_ setups. No clue why.
static void get_clang_resource_dir(char* resource_dir, const int resource_dir_size) {
	FILE* fp = popen("clang -print-resource-dir", "r");
	if (fp && fgets(resource_dir, resource_dir_size, fp)) {
		// Strip newline
		char *nl = strchr(resource_dir, '\n');
		if (nl) *nl = 0;
	}
	pclose(fp);
}

void parse_file(const char* file_path, reflect_ctx* ctx, const char* const* command_line_args, const int command_line_arg_count) {
	/*printf("Command line args: ");
	for (usize i = 0; i < command_line_arg_count; i++) {
		printf("%s ", command_line_args[i]);
	}*/

	// HACK: This should be set automatically, but on some systems it doesn't!
	char resource_dir[4096];
	get_clang_resource_dir(resource_dir, sizeof(resource_dir));
	const char** command_line_args_hack = malloc(sizeof(const char*) * (command_line_arg_count + 2));
	memcpy(command_line_args_hack, command_line_args, sizeof(const char*) * command_line_arg_count);
	command_line_args_hack[command_line_arg_count] = "-resource-dir";
	command_line_args_hack[command_line_arg_count + 1] = resource_dir;
	/*for (usize i = 0; i < command_line_arg_count; i++) {
		command_line_args_hack[i] = command_line_args[i];
	}*/

	CXIndex index = clang_createIndex(0, 0); //Create index
	CXTranslationUnit unit;
	const enum CXErrorCode parse_error = clang_parseTranslationUnit2(
		index,
		file_path, command_line_args_hack, command_line_arg_count+2,
		// file_path, command_line_args, command_line_arg_count,
		nullptr, 0,
		// CXTranslationUnit_SkipFunctionBodies | CXTranslationUnit_IgnoreNonErrorsFromIncludedFiles | CXTranslationUnit_Incomplete,
		// CXTranslationUnit_SkipFunctionBodies | CXTranslationUnit_KeepGoing,
		CXTranslationUnit_SkipFunctionBodies | CXTranslationUnit_IgnoreNonErrorsFromIncludedFiles | CXTranslationUnit_KeepGoing,
		// CXTranslationUnit_None,
		&unit);

	free(command_line_args_hack);

	if (parse_error != CXError_Success) {
		panic("Unable to parse translation unit %s with error code %u. Quitting.", file_path, parse_error);
	}

	for (unsigned i = 0; i < clang_getNumDiagnostics(unit); i++) {
		CXDiagnostic diag = clang_getDiagnostic(unit, i);

		const enum CXDiagnosticSeverity severity = clang_getDiagnosticSeverity(diag);

		const CXString string = clang_formatDiagnostic(diag, clang_defaultDiagnosticDisplayOptions());

		const char* style = severity == CXDiagnostic_Error || severity == CXDiagnostic_Fatal ? ANSI_STYLE(RED) : ANSI_STYLE(YELLOW);
		fprintf(stderr, "%s%s" ANSI_RESET "\n", style, clang_getCString(string));

		if (severity == CXDiagnostic_Error)
			ctx->had_error = true;

		clang_disposeString(string);
		clang_disposeDiagnostic(diag);
	}

	CXCursor cursor = clang_getTranslationUnitCursor(unit); // Obtain a cursor at the root of the translation unit

	clang_visitChildren(cursor, search_for_reflected_types, ctx);

	clang_disposeTranslationUnit(unit);
	clang_disposeIndex(index);
}
