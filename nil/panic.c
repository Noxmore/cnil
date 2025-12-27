#include "panic.h"

// #include "format.h"

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>

nil_panic_hook nil_current_panic_hook;

void $_nil_panic(const char* fn, const char* at, const char* fmt, ...) {
	fprintf(stderr, "PANICKED AT %s (%s): ", at, fn);
	va_list args;
	/*va_start(args, fmt);
	vfprintf(stderr, fmt, args);
	va_end(args);*/

	va_start(args, fmt);
	const int msg_len = vsnprintf(nullptr, 0, fmt, args);
	va_end(args);

	string message;

	if (msg_len < 0) {
		message = EMPTY_STRING;
	} else {
		const usize msg_cap = msg_len + 1;
		char* msg_buf = malloc(msg_cap);

		va_start(args, fmt);
		vsnprintf(msg_buf, msg_cap, fmt, args);
		va_end(args);

		message = (string){
			.data = msg_buf,
			.len = msg_len,
			.cap = msg_cap,
		};
	}

	fputs(message.data, stdout);
	fputc('\n', stderr);

	if (nil_current_panic_hook) {
		nil_current_panic_hook((nil_panic_info){
			.fn = fn,
			.at = at,
			.message = message,
		});
	}

	string_free(&message);

	abort();
}
