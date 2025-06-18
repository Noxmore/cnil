/* CON - C Object Notation

TODO: Document

*/

#pragma once

#include "nint.h"

typedef enum ConTokenType {
	CON_BRACE_LEFT,
	CON_BRACE_RIGHT,
	CON_DOT,
	CON_COMMA,
	CON_IDENT,
	CON_STRING,
	CON_NUMBER,
} ConTokenType;

typedef struct ConToken {
	ConTokenType type;
	// Not null-terminated
	const char* src;
	usize len;
} ConToken;

typedef struct ConLexer {
	char* src;
	usize pos;
} ConLexer;

ConLexer conNewLexer();
void conDropLexer(ConLexer* lex);

// Parses the next token out of a lexer. Returns `false` if failed.
bool conNextToken(ConLexer* lex, ConToken* dst);
