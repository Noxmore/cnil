#include "format.h"

#include <stdarg.h>
#include <stdlib.h>
#include <string.h>

string nil_internal_concat_strings(const usize string_count, ...) {
	va_list args;
	va_start(args);

	usize output_len = 0;
	for (usize i = 0; i < string_count; i++)
		output_len += va_arg(args, str).len;

	va_end(args);

	if (output_len == 0)
		return EMPTY_STRING;

	const usize cap = output_len + 1;
	char* buf = malloc(cap);

	usize buf_cursor = 0;
	va_start(args);

	for (usize i = 0; i < string_count; i++) {
		const str s = va_arg(args, str);
		memcpy(buf + buf_cursor, s.data, s.len);
		buf_cursor += s.len;
	}
	buf[output_len] = '\0';

	va_end(args);

	return (string){
		.data = buf,
		.len = output_len,
		.cap = cap,
	};
}
