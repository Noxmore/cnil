#pragma once

#include "nint.h"

// TODO: I don't think this API is powerful enough, but I don't entirely know how to improve it.
//       Should we have a global allocator?

typedef void(*allocator_free_fn)(void*);

typedef struct allocator {
	void* user_data;
	void* (*alloc)(struct allocator* self, usize byte_count, void* free_fn);
	void* (*realloc)(struct allocator* self, void* ptr, usize byte_count);
	void (*free)(struct allocator* self, void* ptr_to_free);
	void (*destroy)(struct allocator* self);
} allocator;

// Returns an allocator that just forwards allocations to C's default `malloc` and `free`.
// This is very cheap to call, as it doesn't allocate any extra data for the allocator.
allocator allocator_default();

extern _Thread_local allocator nil_global_allocator;

// Allocate memory on the heap with the current global allocator.
void* nil_alloc(usize byte_count);
// Reallocate memory on the heap with the current global allocator.
void* nil_realloc(void* ptr, usize byte_count);
// Free memory allocated by the current global allocator.
void nil_free(void* ptr_to_free);

// A bunch of this is referenced from https://github.com/mtrebi/memory-allocators

// allocator allocator_stack(usize capacity);
// allocator allocator_pool(usize block_capacity, usize blocks);

// Binary tree allocator (sorted based on size)?