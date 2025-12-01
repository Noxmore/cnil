#include "con.h"
#include <stdlib.h>

con_lexer con_lexer_new(const char* src) {
	return (con_lexer){ .src = src, .pos = 0 };
}
/*void con_lexer_drop(con_lexer* lex) {
	free(lex->src);
}*/

bool con_next_token(con_lexer* lex, con_token* dst) {
	char chr = lex->src[lex->pos];

	if (chr == '{') {
		dst->type = con_brace_left;
		dst->src.data = &lex->src[lex->pos];
		dst->src.len = 1;
	} else if (chr == '}') {
		dst->type = con_brace_right;
		dst->src.data = &lex->src[lex->pos];
		dst->src.len = 1;
	} else if (chr == ',') {
		dst->type = con_comma;
		dst->src.data = &lex->src[lex->pos];
		dst->src.len = 1;
	} else if (chr == '.') {
		dst->type = con_dot;
		dst->src.data = &lex->src[lex->pos];
		dst->src.len = 1;
	} else if (chr == '"') {
		dst->type = con_string;
		dst->src.data = &lex->src[lex->pos];
		dst->src.len = 1;
	}

	lex->pos++;
}

void con_write(FILE* file, const type_info* type, void* data) {

}
