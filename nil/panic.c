#include "panic.h"

// #include "format.h"

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>

void $_nil_panic(const char* fn, const char* at, const char* fmt, ...) {
	// format_str(fmt, __builtin_va_arg());
	// fputs(stderr);
	// fprintf(stderr, "PANICKED AT %s (%s): %s\n", at, fn, fmt);
	fprintf(stderr, "PANICKED AT %s (%s): ", at, fn);
	va_list args;
	va_start(args, fmt);
	vfprintf(stderr, fmt, args);
	va_end(args);
	fputc('\n', stderr);
	// fputs(fmt, stderr);
	abort();
}
