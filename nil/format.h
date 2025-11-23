#pragma once

#include "stream.h"

typedef void (*nil_formatter_fn)(write_stream*, void*);

void register_nil_formatter(const char* fmt_code, nil_formatter_fn formatter);

void format_str(write_stream* stream, const char* fmt, ...);