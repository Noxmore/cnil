#pragma once

#include "string.h"

typedef struct write_stream write_stream;

bool stream_write(write_stream* stream, char byte);
bool stream_write_str(write_stream* stream, str s);