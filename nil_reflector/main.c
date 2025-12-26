#include <limits.h>
#include <stdio.h>
#include <stdlib.h>

#include "data.h"
#include "parse.h"
#include "write.h"

#include "nil/ansi_colors.h"
#include "nil/string.h"
#include "nil/reflect.h"

// TODO: How do we handle vectors?

static void print_help() {
	fputs(
		"USAGE: nil_reflector <compiler options> <src> <out>\n",
		stderr
	);
}

int main(const int argc, const char* const* argv) {
	// printf("foo: %lu", type_registry_index(TYPE_INFO(string)));
	if (argc < 3) {
		print_help();
		return 1;
	}

	const char* out_path = argv[argc-1];
	const char* src_path = argv[argc-2];

	// fprintf(stderr, ANSI_STYLE(GREEN, BOLD) "  Reflecting " ANSI_STYLE(WHITE) "%s " ANSI_STYLE(BRIGHT_BLACK) "(%s)" ANSI_RESET "\n", cstr_get_filename(src_path), cstr_get_filename(out_path));

	reflect_ctx ctx = {0};

	char canonical_file_path[PATH_MAX];
	realpath(src_path, canonical_file_path);
	ctx.canonical_file_path = canonical_file_path;

	// Parse reflected types out of the header file.
	parse_file(src_path, &ctx, argv + 1, argc - 3);

	if (ctx.had_error) {
		reflect_ctx_free(&ctx);
		return 1;
	}

	// Put the definitions into an implementation file.
	FILE* output = fopen(out_path, "w");

	if (output == nullptr) {
		print_help();
		fprintf(stderr, ANSI_STYLE(RED) "Failed to open \"%s\" for writing.\n", out_path);
		return 1;
	}

	write_parsed_types(output, &ctx, src_path);

	reflect_ctx_free(&ctx);
}