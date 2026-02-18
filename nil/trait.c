#include "trait.h"

#include <string.h>

#include "hashtable.h"
#include "panic.h"
#include "vec.h"

// We currently use a void* indirection here, but we technically don't need to, it just sounds hard implementing a
// dynamic hashmap with cwisstable. Maybe some other time.
typedef struct trait_registration {
	bool owned;
	// If this is nullptr, the type doesn't have a trait implementation.
	void* data;
} trait_registration;

// CWISS_DECLARE_FLAT_HASHMAP(trait_cache, const type_info*, trait_registration);

// TODO: Handle multithreading with mutexes.
typedef struct trait_registry_inner {
	usize size, align;
	hashmap_anon(const type_info*, trait_registration) cache;
	vec(trait_recognizer) recognizers;
	nil_free_fn free;
	arena_allocator arena; // Nothing really gets freed after the fact, so an arena is a no-brainer.
	_Atomic bool locked;
} trait_registry_inner;

static_assert(sizeof(trait_registry_inner) == sizeof(trait_registry));
static_assert(alignof(trait_registry_inner) == alignof(trait_registry));

// Probably doesn't need to be atomic, but why not.
static _Atomic usize total_implementations = 0;

static void lock_registry(trait_registry_inner* self) {
	if (self->locked)
		panic("Trait registry locked! Are you multithreading? It isn't supported yet!");
	self->locked = true;
}

static void unlock_registry(trait_registry_inner* self) { self->locked = false; }

void trait_registry_init(trait_registry* registry, const usize size, const usize align, const nil_free_fn free) {
	trait_registry_inner* self = (trait_registry_inner*) registry;
	lock_registry(self);
	self->size = size;
	self->align = align;
	self->cache = (typeof(self->cache)){0};
	self->free = free;
	self->arena = arena_new();
	unlock_registry(self);
}

void trait_registry_free(trait_registry* registry) {
	trait_registry_inner* self = (trait_registry_inner*) registry;
	lock_registry(self);
	/*auto iter = trait_cache_iter(&self->cache);
	for (trait_cache_Entry* entry = trait_cache_Iter_get(&iter); entry != nullptr; entry = trait_cache_Iter_next(&iter)) {
		if (self->free)
			self->free(entry->val.data);

		if (entry->val.owned)
			free(entry->val.data);
	}
	trait_cache_destroy(&self->cache);*/

	hashtable_foreach_mut(entry, &self->cache) {
		if (self->free)
			self->free(entry->value.data);

		// if (entry->value.owned)
		// 	free(entry->value.data);
	}
	/*hashmap_free(&self.)

	vec_free(&self->recognizers, arena_ref(&self->arena));*/

	arena_destroy(&self->arena);

	// We don't unlock because this registry shouldn't be used anymore!
}

void trait_registry_impl_recognizer(trait_registry* registry, const trait_recognizer recognizer) {
	trait_registry_inner* self = (trait_registry_inner*) registry;
	lock_registry(self);
	vec_push(&self->recognizers, arena_ref(&self->arena), recognizer);
	unlock_registry(self);
}

void trait_registry_impl_owned(trait_registry* registry, const type_info* type, const void* trait_data) {
	trait_registry_inner* self = (trait_registry_inner*) registry;
	lock_registry(self);
	void* data = arena_alloc(&self->arena, nullptr, type->align, 0, type->size);
	memcpy(data, trait_data, self->size);

	hashmap_insert(&self->cache, arena_ref(&self->arena), type, { .owned = true, .data = data });

	/*trait_cache_insert(&self->cache, &(trait_cache_Entry){
		.key = type,
		.val = {
			.owned = true,
			.data = data,
		},
	});*/
	total_implementations++;
	unlock_registry(self);
}

void trait_registry_impl_static(trait_registry* registry, const type_info* type, void* trait_data) {
	trait_registry_inner* self = (trait_registry_inner*) registry;
	lock_registry(self);
	hashmap_insert(&self->cache, arena_ref(&self->arena), type, { .owned = false, .data = trait_data });
	/*trait_cache_insert(&self->cache, &(trait_cache_Entry){
		.key = type,
		.val = {
			.owned = false,
			.data = trait_data,
		},
	});*/
	total_implementations++;
	unlock_registry(self);
}

const void* trait_registry_get(trait_registry* registry, const type_info* type) {
	trait_registry_inner* self = (trait_registry_inner*) registry;

	// This helps with possible stack overflows as two recognizers try to get back and forth.
	if (self->locked)
		return nullptr;
	self->locked = true;

	// If we have the registration cached, just return that.
	/*const auto iter = trait_cache_cfind(&self->cache, &type);
	const auto entry = trait_cache_CIter_get(&iter);*/
	const auto registration = hashmap_get(&self->cache, &type);

	if (registration != nullptr) {
		self->locked = false;
		return registration->data;
	}

	// If we don't, run recognizers.
	// void* data = aligned_alloc(self->align, self->size);
	void* data = arena_alloc(&self->arena, nullptr, self->align, 0, self->size);
	for (usize i = 0; i < self->recognizers.len; i++) {
		if (self->recognizers.data[i](type, data)) {
			hashmap_insert(&self->cache, arena_ref(&self->arena), type, { .owned = true, .data = data });
			/*trait_cache_insert(&self->cache, &(trait_cache_Entry){
				.key = type,
				.val = {
					.owned = true,
					.data = data,
				},
			});*/
			total_implementations++;
			self->locked = false;
			return data;
		}
	}

	// Recognizers didn't find anything, there's no trait, let's cache that.
	// free(data);
	arena_alloc(&self->arena, data, self->align, self->size, 0);
	hashmap_insert(&self->cache, arena_ref(&self->arena), type, { .owned = false, .data = nullptr });
	/*trait_cache_insert(&self->cache, &(trait_cache_Entry){
		.key = type,
		.val = {
			.owned = false,
			.data = nullptr,
		},
	});
	*/

	self->locked = false;
	return nullptr;
}

usize total_implemented_traits() {
	return total_implementations;
}


// -- COMMON TRAITS -- //

DEFINE_TRAIT(destructor_trait, nullptr)
DEFINE_TRAIT(default_trait, nullptr)
DEFINE_TRAIT(clone_trait, nullptr)
DEFINE_TRAIT(primitive_conversion_trait, nullptr)
DEFINE_TRAIT(list_trait, nullptr)
