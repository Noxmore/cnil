#include "alloc.h"

#include "internal/sys_alloc.h"
#include "memory_util.h"
#include "vec.h"

#include <assert.h>
#include <stdlib.h>
#include <string.h>

#include "math.h"

static void dummy_allocation_callback(void* ctx, void* ptr, usize align, usize old, usize new) {}
nil_allocation_callback_fn nil_allocation_callback = dummy_allocation_callback;

/*owned_allocator static_allocator = {};
_Atomic(bool) initialized_static_allocator = false;

void nil_set_static_allocator(owned_allocator allocator) {
	if (initialized_static_allocator)
		destroy_allocator(&static_allocator);

	static_allocator = allocator;
	initialized_static_allocator = true;
}
allocator_ref nil_get_static_allocator() {
	if (!initialized_static_allocator)
		static_allocator = mallocator_interface(mallocator_new());

	return static_allocator.ref;
}*/

void* malloc_bridge(void* ctx, void* ptr, usize align, const usize old, const usize new) {
	nil_allocation_callback(ctx, ptr, align, old, new);

	if (ptr) {
		if (new == 0 && old > 0) {
			free(ptr);
			return nullptr;
		}

		ptr = realloc(ptr, new);
		assert(ptr != nullptr);
		return ptr;
	}

	if (new == 0)
		return nullptr;

	return aligned_alloc(align, new);
}

const allocator_ref staticalloc = {
	.alloc = malloc_bridge,
};

/*__attribute__((destructor))
void destroy_static_allocator() {
	if (initialized_static_allocator)
		destroy_allocator(&static_allocator);
}*/

typedef struct arena_block {
	struct arena_block* next;
	void* last_alloc; // For realloc optimization.
	u8* buffer_end;
	u8* head;
	u8 buffer[];
} arena_block;

arena_allocator arena_new() {
	return (arena_allocator){};
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
static void* arena_alloc_recursive(arena_block* block, void* ptr, const usize align, const usize size) {
	u8* from;
	// Realloc optimization: extend the previous one.
	if (block->last_alloc != nullptr && ptr == block->last_alloc)
		from = block->last_alloc;
	else
		from = (u8*)pad_to_align((usize)block->head, align);

	u8* to = from + size;

	// We can't fit it here, let's go to the next block!
	if (to >= block->buffer_end) {
		if (block->next == nullptr)
			block->next = create_arena_block();
		return arena_alloc_recursive(block->next, ptr, align, size);
	}

	block->head = to;

	return from;
}
void* arena_alloc(arena_allocator* arena, void* ptr, const usize align, const usize old, const usize new) {
	nil_allocation_callback(arena, ptr, align, old, new);

	if (new >= max_arena_allocation_size()) {
		void* big_alloc = aligned_alloc(align, new);

		if (ptr)
			memcpy(big_alloc, ptr, old);

		vec_push(&arena->big_allocations, staticalloc, big_alloc);
		return big_alloc;
	}

	if (arena->first_block == nullptr)
		arena->first_block = create_arena_block();

	void* new_ptr = arena_alloc_recursive(arena->first_block, ptr, align, new);

	if (ptr != nullptr && ptr != new_ptr)
		memcpy(new_ptr, ptr, old);

	return new_ptr;
}

static usize arena_allocated_bytes_recursive(const arena_block* block) {
	if (block == nullptr)
		return 0;
	return (block->head - block->buffer) + arena_allocated_bytes_recursive(block->next);
}
usize arena_allocated_bytes(const arena_allocator* arena) {
	return arena_allocated_bytes_recursive(arena->first_block);
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
void arena_destroy(arena_allocator* arena) {
	arena_destroy_recursive(arena, arena->first_block);

	for (usize i = 0; i < arena->big_allocations.len; i++)
		free(arena->big_allocations.data[i]);
	vec_free(&arena->big_allocations, staticalloc);
}

// Arena allocator interface. I use wrapper functions for better compile-time error checking.

static void* arena_interface_alloc(void* ctx, void* ptr, const usize align, const usize old, const usize new) {
	return arena_alloc(ctx, ptr, align, old, new);
}

static void arena_interface_reset(void* ctx) {
	arena_reset(ctx);
}

static void arena_owned_destroy(void* ctx) {
	arena_destroy(ctx);
	free(ctx);
}

allocator_ref arena_ref(arena_allocator* arena) {
	return (allocator_ref){
		.alloc = arena_interface_alloc,
		.ctx = arena,
	};
}
owned_allocator arena_interface(arena_allocator arena) {
	void* ctx = malloc(sizeof(arena_allocator));
	memcpy(ctx, &arena, sizeof(arena_allocator));

	return (owned_allocator){
		.reset = arena_interface_reset,
		.destroy = arena_owned_destroy,
		.ref = arena_ref(&arena),
	};
}

mallocator mallocator_new() {
	return (mallocator){};
}

typedef struct mallocator_metadata {
	usize allocation_idx;
} mallocator_metadata;

void* mallocator_alloc(mallocator* self, void* ptr, usize align, const usize old, const usize new) {
	nil_allocation_callback(self, ptr, align, old, new);

	align = max_usize(align, alignof(usize));
	const usize meta_size = pad_to_align(sizeof(mallocator_metadata), align);

	if (ptr) {
		mallocator_metadata* meta = ptr - meta_size;
		if (new == 0 && old > 0) {
			vec_remove_swap_last(&self->allocations, meta->allocation_idx);
			mallocator_metadata* moved_last_meta = self->allocations.data[meta->allocation_idx] - meta_size;
			moved_last_meta->allocation_idx = meta->allocation_idx;
			free(meta);
			return nullptr;
		}

		meta = realloc(meta, new + meta_size);
		assert(meta != nullptr);
		self->allocations.data[meta->allocation_idx] = meta;
		return (void*)meta + meta_size;
	}

	mallocator_metadata* meta = aligned_alloc(align, new + meta_size);
	meta->allocation_idx = self->allocations.len;
	vec_push(&self->allocations, staticalloc, meta);
	return (void*)meta + meta_size;
}
void mallocator_reset(mallocator* self) {
	for (usize i = 0; i < self->allocations.len; i++)
		free(self->allocations.data[i]);
	vec_clear(&self->allocations);
}
void mallocator_destroy(mallocator* self) {
	mallocator_reset(self);
	vec_free(&self->allocations, staticalloc);
}


static void* mallocator_interface_alloc(void* ctx, void* ptr, const usize align, const usize old, const usize new) {
	return mallocator_alloc(ctx, ptr, align, old, new);
}

static void mallocator_interface_reset(void* ctx) {
	mallocator_reset(ctx);
}

static void mallocator_owned_destroy(void* ctx) {
	mallocator_destroy(ctx);
	free(ctx);
}

allocator_ref mallocator_ref(mallocator* self) {
	return (allocator_ref){
		.alloc = mallocator_interface_alloc,
		.ctx = self,
	};
}
owned_allocator mallocator_interface(mallocator self) {
	void* ctx = malloc(sizeof(mallocator));
	memcpy(ctx, &self, sizeof(mallocator));

	return (owned_allocator){
		.reset = mallocator_interface_reset,
		.destroy = mallocator_owned_destroy,
		.ref = mallocator_ref(&self),
	};
}

void* suballocator_alloc(mallocator* self, void* ptr, usize align, usize old, usize new) {
	nil_allocation_callback(self, ptr, align, old, new);

}

// TODO: If I uncomment this, fix hashset iterators to use a for loop to avoid UB.

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

*//*typedef struct nil_lifetime {
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