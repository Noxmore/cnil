#include "alloc.h"

// #include "internal/cwisstable.h"
#include "internal/sys_alloc.h"
#include "memory_util.h"

#include <assert.h>
#include <stdlib.h>
#include <string.h>

typedef struct arena_block {
	struct arena_block* next;
	u8* buffer_end;
	u8* head;
	u8 buffer[];
} arena_block;


arena_allocator create_arena() {
	return (arena_allocator){0};
}

static usize max_arena_allocation_size() {
	return nil_page_size() - sizeof(arena_block);
}
static arena_block* create_arena_block() {
	const usize page_size = nil_page_size();
	assert(page_size > sizeof(arena_block));

	arena_block* block = nil_reserve_page();
	memset(block, 0, page_size);

	block->buffer_end = (u8*)block + page_size;
	block->head = block->buffer;

	return block;
}
static void* arena_alloc_recursive(arena_block* block, const usize align, const usize size) {
	u8* from = (u8*)pad_to_align((usize)block->head, align);
	u8* to = from + size;

	// We can't fit it here, let's go to the next block!
	if (to >= block->buffer_end) {
		if (block->next == nullptr)
			block->next = create_arena_block();
		return arena_alloc_recursive(block->next, align, size);
	}

	block->head = to;

	return from;
}
void* arena_alloc(arena_allocator* arena, const usize align, const usize size) {
	if (size >= max_arena_allocation_size()) {
		void* ptr = aligned_alloc(align, size);
		vec_push(&arena->big_allocations, ptr);
		return ptr;
	}

	if (arena->first_block == nullptr)
		arena->first_block = create_arena_block();

	return arena_alloc_recursive(arena->first_block, align, size);
}

static void arena_reset_recursive(arena_block* block) {
	if (block == nullptr)
		return;

	arena_reset_recursive(block->next);
	block->head = block->buffer;
}
void arena_reset(arena_allocator* arena) {
	arena_reset_recursive(arena->first_block);

	for (usize i = 0; i < arena->big_allocations.len; i++)
		free(arena->big_allocations.data[i]);
	vec_clear(&arena->big_allocations);
}

static void arena_destroy_recursive(arena_allocator* arena, arena_block* block) {
	if (block == nullptr)
		return;

	arena_destroy_recursive(arena, block->next);
	nil_free_page(block);
	// arena->backing.alloc(arena->backing.ctx, block, arena_block_size(), 0);
}
void arena_destroy(arena_allocator arena) {
	arena_destroy_recursive(&arena, arena.first_block);

	for (usize i = 0; i < arena.big_allocations.len; i++)
		free(arena.big_allocations.data[i]);
	vec_free(&arena.big_allocations);
}

/*
CWISS_DECLARE_FLAT_HASHSET(galloc_ptr_map, void*);

typedef struct galloc_ctx {
	galloc_ptr_map allocations;
} galloc_ctx;


global_allocator create_global_allocator() {
	return (global_allocator){
		.interface = {
			.alloc = (nil_alloc_fn)galloc,
			.ctx = malloc(sizeof(galloc_ctx)),
		}
	};
}
void* galloc(global_allocator* allocator, void* ptr, const usize old, const usize new) {
	galloc_ctx* ctx = allocator->interface.ctx;
	if (new == 0 && ptr != nullptr) {
		assert(galloc_ptr_map_contains(&ctx->allocations, ptr));
		galloc_ptr_map_erase(&ctx->allocations, ptr);
		free(ptr);
		return nullptr;
	}

	galloc_ptr_map_insert(&ctx->allocations, ptr);
	return realloc(ptr, new);
}
void galloc_reset(global_allocator* allocator) {
	galloc_ctx* ctx = allocator->interface.ctx;
	auto iter = galloc_ptr_map_iter(&ctx->allocations);
	galloc_ptr_map_Entry* entry;
	while ((entry = galloc_ptr_map_Iter_next(&iter))) {
		free(*entry);
	}
	galloc_ptr_map_clear(&ctx->allocations);
}
void galloc_destroy(global_allocator allocator) {
	galloc_reset(&allocator);
	galloc_ctx* ctx = allocator.interface.ctx;
	galloc_ptr_map_destroy(&ctx->allocations);
}*/

/*typedef struct arena {
	arena_block *blocks;
} arena;*/



/*nil_allocator create_arena() {
	return (nil_allocator){

	};
}*/

/*typedef struct lifetime_allocation {
	nil_destructor destructor;
	usize size;
	usize count;
} lifetime_allocation;

// TODO: If we're good we could probably convert this to a vector, storing some information next to the allocation itself
CWISS_DECLARE_FLAT_HASHMAP(lifetime_ptr_map, void*, lifetime_allocation);

typedef struct nil_lifetime_inner {
	lifetime_ptr_map map;
	bool initialized;
} nil_lifetime_inner;

static_assert(sizeof(nil_lifetime) == sizeof(nil_lifetime_inner));
static_assert(alignof(nil_lifetime) == alignof(nil_lifetime_inner));

/*typedef struct nil_lifetime {
	lifetime_ptr_map map;
} nil_lifetime;#1#

void* nil_alloc(nil_lifetime* lifetime, void* current, const usize size, const usize count, const nil_destructor destructor) {
	nil_lifetime_inner* inner = (nil_lifetime_inner*)lifetime;

	// Initialize the lifetime.
	if (!inner->initialized) {
		inner->map = lifetime_ptr_map_new(8);
		inner->initialized = true;
	}

	void* new = reallocarray(current, size, count);
	if (new == nullptr)
		panic("Failed to allocate! current = %p, size = %lu, count = %lu, destructor = %p", current, size, count, destructor);
	if (new != current)
		lifetime_ptr_map_erase(&inner->map, current);

	const lifetime_ptr_map_Entry entry = {
		.key = new,
		.val = {
			.destructor = destructor,
			.size = size,
			.count = count,
		},
	};
	lifetime_ptr_map_insert(&inner->map, &entry);

	return new;
}

void nil_free(nil_lifetime* lifetime, void* ptr) {\
	nil_lifetime_inner* inner = (nil_lifetime_inner*)lifetime;
	assert(lifetime_ptr_map_contains(&inner->map, ptr));

	const auto iter = lifetime_ptr_map_find(&inner->map, ptr);
	const auto entry = lifetime_ptr_map_Iter_get(&iter);
	if (entry != nullptr) {
		if (entry->val.destructor) for (usize i = 0; i < entry->val.count; i++)
			entry->val.destructor(ptr + entry->val.size * i);
	}

	lifetime_ptr_map_erase(&inner->map, ptr);
	free(ptr);
}

void nil_release(nil_lifetime* lifetime) {
	nil_lifetime_inner* inner = (nil_lifetime_inner*)lifetime;

	auto iter = lifetime_ptr_map_iter(&inner->map);
	lifetime_ptr_map_Entry* entry;
	while (entry = lifetime_ptr_map_Iter_next(&iter)) {
		if (entry->val.destructor) for (usize i = 0; i < entry->val.count; i++)
			entry->val.destructor(entry->key + entry->val.size * i);
		free(entry->key);
	}

	lifetime_ptr_map_destroy(&inner->map);
}*/

/*typedef struct lifetime_allocation {
	nil_destructor destructor;
	usize size;
	usize count;
} lifetime_allocation;*/

/*typedef struct nil_lifetime {
	lifetime_ptr_map map;
} nil_lifetime;*/

/*void* nil_alloc(nil_lifetime* lifetime, void* current, const usize size, const usize count, const nil_destructor destructor) {
	// Initialization.

	void* new = reallocarray(current, size, count);
	if (new == nullptr)
		panic("Failed to allocate! current = %p, size = %lu, count = %lu, destructor = %p", current, size, count, destructor);
	if (new != current)
		lifetime_ptr_map_erase(&inner->map, current);

	const lifetime_ptr_map_Entry entry = {
		.key = new,
		.val = {
			.destructor = destructor,
			.size = size,
			.count = count,
		},
	};
	lifetime_ptr_map_insert(&inner->map, &entry);

	return new;
}

void nil_free(nil_lifetime* lifetime, void* ptr) {\
	nil_lifetime_inner* inner = (nil_lifetime_inner*)lifetime;
	assert(lifetime_ptr_map_contains(&inner->map, ptr));

	const auto iter = lifetime_ptr_map_find(&inner->map, ptr);
	const auto entry = lifetime_ptr_map_Iter_get(&iter);
	if (entry != nullptr) {
		if (entry->val.destructor) for (usize i = 0; i < entry->val.count; i++)
			entry->val.destructor(ptr + entry->val.size * i);
	}

	lifetime_ptr_map_erase(&inner->map, ptr);
	free(ptr);
}

void nil_release(nil_lifetime* lifetime) {
	nil_lifetime_inner* inner = (nil_lifetime_inner*)lifetime;

	auto iter = lifetime_ptr_map_iter(&inner->map);
	lifetime_ptr_map_Entry* entry;
	while (entry = lifetime_ptr_map_Iter_next(&iter)) {
		if (entry->val.destructor) for (usize i = 0; i < entry->val.count; i++)
			entry->val.destructor(entry->key + entry->val.size * i);
		free(entry->key);
	}

	lifetime_ptr_map_destroy(&inner->map);
}*/