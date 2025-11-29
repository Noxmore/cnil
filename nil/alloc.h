// WIP custom allocator API

#pragma once

#include "nint.h"

typedef void* (*nil_alloc_fn)(void* ctx, void* ptr, usize old, usize new);

/*typedef struct nil_allocator {
	nil_alloc_fn alloc_fn;
	void (*free)(struct nil_allocator*);
} nil_allocator;*/

typedef struct arena_block {
	struct arena_block *next;
	u8 *buffer_end;
	u8 *head;
	u8 buffer[];
} arena_block;

typedef struct arena_allocator {
	arena_block* first_block;
	nil_alloc_fn interface;
} arena_allocator;

arena_allocator create_arena();
void arena_reset(arena_allocator* arena);
void arena_destroy(arena_allocator* arena);

/*typedef struct block_allocator {

} block_allocator;*/

// static inline void nil_free(nil_allocator)
