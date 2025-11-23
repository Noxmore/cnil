#include "parse.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <clang-c/Index.h>

#include "nil/panic.h"
#include "nil/ansi_colors.h"

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

static bool parse_ctx_contains_type(const reflect_ctx* ctx, CXCursor cursor) {
	for (usize i = 0; i < ctx->types.len; i++) {
		const type_info_builder* type = &ctx->types.data[i];
		if (is_display_name_eq(cursor, type->name.data) && cursor_kind_to_type_info_kind(clang_getCursorKind(cursor)) == type->kind)
			return true;
	}

	return false;
}

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
		if (is_display_name_eq(cursor, NIL_ANNOTATION_REFLECT))
			return CXChildVisit_Continue;

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
			if (field_type.kind == CXType_Record || field_type.kind == CXType_Enum)
				field.field_type = cursor_spelling(clang_getTypeDeclaration(field_type));
			else
				field.field_type = convert_str(clang_getTypeSpelling(field_type));
			// field.field_type = convert_str(clang_getTypeSpelling(field_type));
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
	if (clang_getCursorKind(cursor) == CXCursor_AnnotateAttr && is_display_name_eq(cursor, NIL_ANNOTATION_REFLECT)) {
		*(bool*)client_data = true;
		return CXChildVisit_Break;
	}

	return CXChildVisit_Continue;
}

static enum CXChildVisitResult search_for_reflected_types(CXCursor cursor, CXCursor parent, CXClientData client_data) {
	// clang_getCursor
	// Allocate a CXString representing the name of the current cursor

	const enum CXCursorKind kind = clang_getCursorKind(cursor);

	if (kind == CXCursor_StructDecl || kind == CXCursor_EnumDecl || kind == CXCursor_UnionDecl) {
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
	}

	return CXChildVisit_Recurse;
}

void parse_file(const char* file_path, reflect_ctx* ctx, const char* const* command_line_args, const int command_line_arg_count) {
	CXIndex index = clang_createIndex(0, 0); //Create index
	CXTranslationUnit unit;
	const enum CXErrorCode parse_error = clang_parseTranslationUnit2(
		index,
		file_path, command_line_args, command_line_arg_count,
		nullptr, 0,
		CXTranslationUnit_SkipFunctionBodies,
		&unit);

	if (parse_error != CXError_Success) {
		panic("Unable to parse translation unit %s with error code %u. Quitting.", file_path, parse_error);
	}

	for (unsigned i = 0; i < clang_getNumDiagnostics(unit); i++) {
		CXDiagnostic diag = clang_getDiagnostic(unit, i);

		const enum CXDiagnosticSeverity severity = clang_getDiagnosticSeverity(diag);

		const CXString string = clang_formatDiagnostic(diag, clang_defaultDiagnosticDisplayOptions());

		if (severity == CXDiagnostic_Error || severity == CXDiagnostic_Fatal) {
			fprintf(stderr, ANSI_STYLE(RED) "ERROR: ");
		} else if (severity == CXDiagnostic_Warning) {
			fprintf(stderr, ANSI_STYLE(YELLOW) "WARNING: ");
		}
		fprintf(stderr, "%s" ANSI_RESET "\n", clang_getCString(string));


		clang_disposeString(string);
		clang_disposeDiagnostic(diag);
	}

	CXCursor cursor = clang_getTranslationUnitCursor(unit); // Obtain a cursor at the root of the translation unit

	clang_visitChildren(cursor, search_for_reflected_types, ctx);

	clang_disposeTranslationUnit(unit);
	clang_disposeIndex(index);
}
