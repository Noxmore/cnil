// WIP custom allocator API

#pragma once

#include "nint.h"
#include "vec.h"
// #include "macro_utils.h"

typedef struct arena_allocator {
	void* first_block;
	// For allocations bigger than the page size, we allocate normally.
	vec_anon(void*) big_allocations;
} arena_allocator;

arena_allocator create_arena();
void* arena_alloc(arena_allocator* arena, usize align, usize size);
void arena_reset(arena_allocator* arena);
void arena_destroy(arena_allocator arena);

/*#ifndef NIL_ALLOCATOR_PREFIX
#define NIL_ALLOCATOR_PREFIX
#endif

#define NIL_ALLOCATOR_API(NAME) NIL_CONCAT_2(NIL_ALLOCATOR_PREFIX, NAME)*/

/*typedef void* (*nil_alloc_fn)(void* ctx, void* ptr, usize old, usize new);
// typedef void* (*nil_alloc_fn)(void* ctx, void* ptr, usize size);

#define alloc_create(ALLOCATOR, T) do { nil_allocator _allocator = (ALLOCATOR); _allocator.alloc(_allocator.ctx, nullptr, 0, sizeof(T)) } while (false)
#define alloc_destroy(ALLOCATOR, PTR) do { nil_allocator _allocator = (ALLOCATOR); _allocator.alloc(_allocator.ctx, (PTR), sizeof(*(PTR)), 0) } while (false)

typedef struct nil_allocator {
	nil_alloc_fn alloc;
	void* ctx;
} nil_allocator;

//////////////////////////////////////////////////////////////////////////////////
//// ARENA
//////////////////////////////////////////////////////////////////////////////////



//////////////////////////////////////////////////////////////////////////////////
//// GLOBAL
//////////////////////////////////////////////////////////////////////////////////

typedef struct global_allocator {
	nil_allocator interface;
} global_allocator;

global_allocator create_global_allocator();
void* galloc(global_allocator* allocator, void* ptr, usize old, usize new);
void galloc_reset(global_allocator* allocator);
void galloc_destroy(global_allocator allocator);*/

/*typedef struct block_allocator {

} block_allocator;*/

// static inline void nil_free(nil_allocator)

// typedef struct nil_lifetime nil_lifetime;

/*// Object that keeps track of allocations, allowing bulk frees.
typedef struct nil_lifetime {
	// THE FORBIDDEN ENCAPSULATION TECHNIQUE
	// usize private[6]; // Private stack-allocated data.

	// Internally, a lifetime shares the structure of a vector.
	void** data;
	usize len;
	usize cap;
} nil_lifetime;
typedef void (*nil_destructor)(void*);
void* nil_alloc(nil_lifetime* lifetime, void* current, usize size, usize count, nil_destructor destructor);
void nil_free(nil_lifetime* lifetime, void* ptr);
void nil_release(nil_lifetime* lifetime);*/
