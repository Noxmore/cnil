#pragma once

#include "nint.h"

// TODO: I don't think this API is powerful enough, but I don't entirely know how to improve it.
//       Should we have a global allocator?

typedef struct allocator {
	void* user_data;
	void* (*alloc)(struct allocator* self, usize byte_count);
	void (*free)(struct allocator* self, void* ptr_to_free);
} allocator;

allocator create_default_allocator();

extern _Thread_local allocator nil_current_allocator;

void* nil_alloc(usize byte_count);
void nil_free(void* ptr_to_free);