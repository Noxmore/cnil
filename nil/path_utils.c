#include "path_utils.h"

const char* nil_get_filename(const char* path) {
	const char* previous_slash = path;

	while (*path != '\0') {
		if (*path == '/')
			previous_slash = path;
		path++;
	}

	return previous_slash+1;
}