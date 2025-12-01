#pragma once

#include "string.h"

typedef enum stream_type : u8 {
	stream_file,
	stream_mem,
} stream_type;

typedef struct write_stream {
	stream_type type;
	u32 cursor;
	union {
		FILE* file;
		char* mem;
	};
} write_stream;

typedef struct read_stream {
	stream_type type;
	u32 cursor;
	union {
		FILE* file;
		const char* mem;
	};
} read_stream;

bool stream_write(write_stream* stream, u8 byte);
bool stream_write_str(write_stream* stream, str s);