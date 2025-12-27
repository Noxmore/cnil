#include "panic.h"

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>

nil_panic_hook nil_current_panic_hook;

void $_nil_panic(const char* fn, const char* at, const char* fmt, ...) {
	fprintf(stderr, "PANICKED AT %s (%s): ", at, fn);
	va_list args;

	va_start(args, fmt);
	const int msg_len = vsnprintf(nullptr, 0, fmt, args);
	va_end(args);

	string message;

	if (msg_len < 0) {
		message = EMPTY_STRING;
	} else {
		message = string_sized(msg_len);

		va_start(args, fmt);
		vsnprintf(message.data, message.cap, fmt, args);
		va_end(args);
	}

	// TODO: debugger kinda reports message.data being correct?? But it doesn't print it out??
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
