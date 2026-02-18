#pragma once

#include "nint.h"
#include "internal/vec_types.h"

// `malloc`, `realloc`, and `free` all combined into one function.
// When `new` is 0, should return `nullptr`. Otherwise, should always produce valid pointer. Failed allocations should crash. TODO: Do we want this behavior?
typedef void* (*nil_alloc_fn)(void* ctx, void* ptr, usize align, usize old, usize new);
typedef void (*nil_allocation_callback_fn)(void* ctx, void* ptr, usize align, usize old, usize new);

// Acts like a fat pointer
typedef struct allocator_ref {
	nil_alloc_fn alloc;
	void* ctx;
} allocator_ref;

typedef struct owned_allocator {
	allocator_ref ref;
	void (*reset)(void* ctx);
	void (*destroy)(void* ctx);
} owned_allocator;

static inline void reset_allocator(owned_allocator* allocator) {
	allocator->reset(allocator->ref.ctx);
}

static inline void destroy_allocator(owned_allocator* allocator) {
	allocator->destroy(allocator->ref.ctx);
}

// Should never be `nullptr`.
extern nil_allocation_callback_fn nil_allocation_callback;

// A raw allocator for malloc. You usually shouldn't use this.
extern const allocator_ref staticalloc;

/*// Warning: Destroys the current static allocator, make sure you only do this in places you know nothing is allocated.
void nil_set_static_allocator(owned_allocator allocator);
allocator_ref nil_get_static_allocator();

// Retrieves a raw allocator for malloc. This doesn't record allocations. You should use `staticalloc` over this unless you know what you're doing.
allocator_ref nil_get_malloc_allocator();

#define staticalloc nil_get_static_allocator()*/


// #define defalloc nil_default_static_allocator.ref

// Used for macros to not duplicate allocator reference expressions.
static inline void* nil_alloc(allocator_ref allocator, void* ptr, const usize align, const usize old, const usize new) {
	return allocator.alloc(allocator.ctx, ptr, align, old, new);
}
#define nil_new(ALLOC, T) nil_alloc(ALLOC, nullptr, alignof(T), 0, sizeof(T))
#define nil_new_array(ALLOC, T, COUNT) nil_alloc(ALLOC, nullptr, alignof(T), 0, sizeof(T) * (COUNT))
#define nil_free(ALLOC, T, PTR) nil_alloc(ALLOC, PTR, alignof(T), sizeof(T), 0)
#define nil_free_array(ALLOC, T, PTR, COUNT) nil_alloc(ALLOC, PTR, alignof(T), sizeof(T) * (COUNT), 0)

// typedef const allocator* allocator_ref;

// ============================================================================================================================== //
//                                                              ARENA                                                             //
// ============================================================================================================================== //

// Arena allocator that directly uses OS pages, removing malloc overhead.
typedef struct arena_allocator {
	void* first_block;
	// For allocations bigger than the page size, we use malloc.
	vec_anon(void*) big_allocations;
} arena_allocator;

arena_allocator arena_new();
void* arena_alloc(arena_allocator* arena, void* ptr, usize align, usize old, usize new);
usize arena_allocated_bytes(const arena_allocator* arena);
void arena_reset(arena_allocator* arena);
void arena_destroy(arena_allocator* arena);

allocator_ref arena_ref(arena_allocator* arena);
owned_allocator arena_interface(arena_allocator arena);


// ============================================================================================================================== //
//                                                           MALLOCATOR                                                           //
// ============================================================================================================================== //

// Tracked malloc.
typedef struct mallocator {
	// This should not be considered public API.
	vec_anon(void*) allocations;
} mallocator;
mallocator mallocator_new();
void* mallocator_alloc(mallocator* self, void* ptr, usize align, usize old, usize new);
void mallocator_reset(mallocator* self);
void mallocator_destroy(mallocator* self);

allocator_ref mallocator_ref(mallocator* self);
owned_allocator mallocator_interface(mallocator self);

// ============================================================================================================================== //
//                                                            LIFETIME                                                            //
// ============================================================================================================================== //

// Tracks allocations to form a sub-lifetime, allowing you to free them all at once if things go awry.
// Freeing is currently an O(n) operation.
typedef struct suballocator {
	allocator_ref super;
	vec_anon(void*) allocations;
} suballocator;
void* suballocator_alloc(mallocator* self, void* ptr, usize align, usize old, usize new);
void suballocator_reset(mallocator* self);
// Releases the allocations back to the backing allocator.
void suballocator_release(mallocator* self);

allocator_ref suballocator_ref(mallocator* self);