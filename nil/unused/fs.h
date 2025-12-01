#pragma once

#include "string.h"

typedef struct fs_file fs_file;

typedef enum fs_flags {
	fs_none = 0,
	fs_read = 1 << 0,
	fs_write = 1 << 1,
} fs_flags;

fs_file* fs_open(str path, fs_flags flags);

void fs_close(fs_file* file);