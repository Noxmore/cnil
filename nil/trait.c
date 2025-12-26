#include "trait.h"

#include "internal/cwisstable.h"
#include "panic.h"
#include "vec.h"

// We currently use a void* indirection here, but we technically don't need to, it just sounds hard implementing a dynamic hashmap with cwisstable.
// Maybe some other time.
typedef struct trait_registration {
	bool owned;
	// If this is nullptr, the type doesn't have a trait implementation.
	void* data;
} trait_registration;

CWISS_DECLARE_FLAT_HASHMAP(trait_cache, const type_info*, trait_registration);

// TODO: Handle multithreading with mutexes.
typedef struct trait_registry {
	usize size, align;
	trait_cache cache;
	vec(trait_recognizer) recognizers;
	nil_free_fn free;
	_Atomic bool locked;
} trait_registry;

static_assert(sizeof(trait_registry) == sizeof(trait_registry(void)));
static_assert(alignof(trait_registry) == alignof(trait_registry(void)));

static void lock_registry(trait_registry* registry) {
	if (registry->locked)
		panic("Trait registry locked! Are you multithreading? It isn't supported yet!");
	registry->locked = true;
}

static void unlock_registry(trait_registry* registry) {
	registry->locked = false;
}

void trait_registry_init(trait_registry* registry, const usize size, const usize align, const nil_free_fn free) {
	lock_registry(registry);
	registry->size = size;
	registry->align = align;
	registry->cache = trait_cache_new(8);
	registry->free = free;
	unlock_registry(registry);
}

void trait_registry_free(trait_registry* registry) {
	lock_registry(registry);
	auto iter = trait_cache_iter(&registry->cache);
	trait_cache_Entry* entry;
	while ((entry = trait_cache_Iter_next(&iter))) {
		if (registry->free)
			registry->free(entry->val.data);

		if (entry->val.owned)
			free(entry->val.data);
	}
	trait_cache_destroy(&registry->cache);

	vec_free(&registry->recognizers);
	// We don't unlock because this registry shouldn't be used anymore!
}

void trait_registry_impl_recognizer(trait_registry* registry, const trait_recognizer recognizer) {
	lock_registry(registry);
	vec_push(&registry->recognizers, recognizer);
	unlock_registry(registry);
}

void trait_registry_impl_owned(trait_registry* registry, const type_info* type, const void* trait_data) {
	lock_registry(registry);
	void* data = aligned_alloc(registry->align, registry->size);
	memcpy(data, trait_data, registry->size);

	trait_cache_insert(&registry->cache, &(trait_cache_Entry){
		.key = type,
		.val = {
			.owned = true,
			.data = data,
		},
	});
	unlock_registry(registry);
}

void trait_registry_impl_static(trait_registry* registry, const type_info* type, void* trait_data) {
	lock_registry(registry);
	trait_cache_insert(&registry->cache, &(trait_cache_Entry){
		.key = type,
		.val = {
			.owned = false,
			.data = trait_data,
		},
	});
	unlock_registry(registry);
}

const void* trait_registry_get(trait_registry* registry, const type_info* type) {
	// This helps with possible stack overflows as two recognizers try to get back and forth.
	if (registry->locked)
		return nullptr;
	registry->locked = true;

	// If we have the registration cached, just return that.
	const auto iter = trait_cache_cfind(&registry->cache, &type);
	const auto entry = trait_cache_CIter_get(&iter);

	if (entry != nullptr) {
		registry->locked = false;
		return entry->val.data;
	}

	// If we don't, run recognizers.
	void* data = aligned_alloc(registry->align, registry->size); // Allocating is fine performance-wise because this is lazy.
	for (usize i = 0; i < registry->recognizers.len; i++) {
		if (registry->recognizers.data[i](type, data)) {
			trait_cache_insert(&registry->cache, &(trait_cache_Entry){
				.key = type,
				.val = {
					.owned = true,
					.data = data,
				},
			});
			registry->locked = false;
			return data;
		}
	}

	// Recognizers didn't find anything, there's no trait, let's cache that.
	free(data);
	trait_cache_insert(&registry->cache, &(trait_cache_Entry){
		.key = type,
		.val = {
			.owned = false,
			.data = nullptr,
		},
	});

	registry->locked = false;
	return nullptr;
}
