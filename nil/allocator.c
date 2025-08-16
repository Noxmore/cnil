#include "allocator.h"

#include <stdlib.h>

static void* default_alloc(allocator* self, usize byte_count) {
	return malloc(byte_count);
}

static void default_free(allocator* self, void* ptr_to_free) {
	free(ptr_to_free);
}

allocator create_default_allocator() {
	return (allocator){
		.user_data = nullptr,
		.alloc = default_alloc,
		.free = default_free,
	};
}

_Thread_local allocator nil_current_allocator = {
	.user_data = nullptr,
	.alloc = default_alloc,
	.free = default_free,
};

void* nil_alloc(const usize byte_count) {
	return nil_current_allocator.alloc(&nil_current_allocator, byte_count);
}

void nil_free(void* ptr_to_free) {
	nil_current_allocator.free(&nil_current_allocator, ptr_to_free);
}
