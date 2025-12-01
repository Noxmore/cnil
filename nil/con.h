/* CON - C Object Notation

This is a data storage format meant to look like C.
Here are some examples:

.entities = {
	{
		.name = "Player",
		.transform = {
			.translation = { 1, 2, 3 },
			.rotation = { },
			.foo = "hello there!",
		},
		.scene_ref = {
			.id = "rune:obj/player",
		},
		.children = { 32, 11, 52 },
	},
}



*/

#pragma once

#include "nint.h"
#include "reflect.h"

typedef enum con_token_type {
	con_brace_left,
	con_brace_right,
	con_dot,
	con_comma,
	con_ident,
	con_string,
	con_number,
} con_token_type;

typedef struct con_token {
	con_token_type type;
	str src;
} con_token;

typedef struct con_lexer {
	const char* src;
	usize pos;
} con_lexer;

con_lexer con_lexer_new(const char* src);
// void con_lexer_drop(con_lexer* lex);

// Parses the next token out of a lexer. Returns `false` if failed.
bool con_next_token(con_lexer* lex, con_token* dst);

// Writes CON to a file.
void con_write(FILE* file, const type_info* type, void* data);
