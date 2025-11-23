#include "stream.h"

typedef struct write_stream {
	enum write_stream_type : u8 {
		write_stream_file,
		write_stream_mem,
	} type;
} write_stream;
