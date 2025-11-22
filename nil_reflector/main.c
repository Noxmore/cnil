#include <stdio.h>

#include "data.h"
#include "parse.h"
#include "write.h"

#include "nil/ansi_colors.h"

// TODO: How do we handle vectors?

static void print_help() {
	fputs(
		"USAGE: nil_reflector <compiler options> <src> <out>\n",
		stderr
	);
}

int main(const int argc, const char* const* argv) {
	if (argc < 3) {
		print_help();
		return 1;
	}

	/*for (int file_idx = 1; file_idx < argc; file_idx++) {
		const char* file_path = argv[file_idx];

	}*/

	const char* out_path = argv[argc-1];
	const char* src_path = argv[argc-2];

	// printf(ANSI_STYLE(GREEN, BOLD) "  Reflecting " ANSI_RESET "%s -> %s\n", src_path, out_path);


	reflect_ctx ctx = {0};

	// Parse reflected types out of the header file.
	parse_file(src_path, &ctx, argv + 1, argc - 3);

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