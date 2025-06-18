#include "con.h"
#include <stdlib.h>

ConLexer conNewLexer() {

}
void conDropLexer(ConLexer* lex) {
	free(lex->src);
}

bool conNextToken(ConLexer* lex, ConToken* dst) {
	char chr = lex->src[lex->pos];
	if (chr == '{') {
		dst->type = CON_BRACE_LEFT;
		dst->src = &lex->src[lex->pos];
		dst->len = 1;
	} else if (chr == '}') {
		dst->type = CON_BRACE_RIGHT;
		dst->src = &lex->src[lex->pos];
		dst->len = 1;
	} else if (chr == ',') {
		dst->type = CON_COMMA;
		dst->src = &lex->src[lex->pos];
		dst->len = 1;
	} else if (chr == '.') {
		dst->type = CON_DOT;
		dst->src = &lex->src[lex->pos];
		dst->len = 1;
	} else if (chr == '"') {
		dst->type = CON_STRING;
		dst->src = &lex->src[lex->pos];
		dst->len = 1;
	}

	lex->pos++;
}
