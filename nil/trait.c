#include "trait.h"

#include "internal/cwisstable.h"
#include "panic.h"
#include "vec.h"

// We currently use a void* indirection here, but we technically don't need to, it just sounds hard implementing a
// dynamic hashmap with cwisstable. Maybe some other time.
typedef struct trait_registration {
	bool owned;
	// If this is nullptr, the type doesn't have a trait implementation.
	void* data;
} trait_registration;

CWISS_DECLARE_FLAT_HASHMAP(trait_cache, const type_info*, trait_registration);

// TODO: Handle multithreading with mutexes.
typedef struct trait_registry_inner {
	usize size, align;
	trait_cache cache;
	vec(trait_recognizer) recognizers;
	nil_free_fn free;
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
	self->cache = trait_cache_new(8);
	self->free = free;
	unlock_registry(self);
}

void trait_registry_free(trait_registry* registry) {
	trait_registry_inner* self = (trait_registry_inner*) registry;
	lock_registry(self);
	auto iter = trait_cache_iter(&self->cache);
	for (trait_cache_Entry* entry = trait_cache_Iter_get(&iter); entry != nullptr; entry = trait_cache_Iter_next(&iter)) {
		if (self->free)
			self->free(entry->val.data);

		if (entry->val.owned)
			free(entry->val.data);
	}
	trait_cache_destroy(&self->cache);

	vec_free(&self->recognizers, staticalloc);
	// We don't unlock because this registry shouldn't be used anymore!
}

void trait_registry_impl_recognizer(trait_registry* registry, const trait_recognizer recognizer) {
	trait_registry_inner* self = (trait_registry_inner*) registry;
	lock_registry(self);
	vec_push(&self->recognizers, staticalloc, recognizer);
	unlock_registry(self);
}

void trait_registry_impl_owned(trait_registry* registry, const type_info* type, const void* trait_data) {
	trait_registry_inner* self = (trait_registry_inner*) registry;
	lock_registry(self);
	void* data = aligned_alloc(self->align, self->size);
	memcpy(data, trait_data, self->size);

	trait_cache_insert(&self->cache, &(trait_cache_Entry){
		.key = type,
		.val = {
			.owned = true,
			.data = data,
		},
	});
	total_implementations++;
	unlock_registry(self);
}

void trait_registry_impl_static(trait_registry* registry, const type_info* type, void* trait_data) {
	trait_registry_inner* self = (trait_registry_inner*) registry;
	lock_registry(self);
	trait_cache_insert(&self->cache, &(trait_cache_Entry){
		.key = type,
		.val = {
			.owned = false,
			.data = trait_data,
		},
	});
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
	const auto iter = trait_cache_cfind(&self->cache, &type);
	const auto entry = trait_cache_CIter_get(&iter);

	if (entry != nullptr) {
		self->locked = false;
		return entry->val.data;
	}

	// If we don't, run recognizers.
	void* data = aligned_alloc(self->align, self->size); // Allocating is fine performance-wise because this is lazy.
	for (usize i = 0; i < self->recognizers.len; i++) {
		if (self->recognizers.data[i](type, data)) {
			trait_cache_insert(&self->cache, &(trait_cache_Entry){
				.key = type,
				.val = {
					.owned = true,
					.data = data,
				},
			});
			total_implementations++;
			self->locked = false;
			return data;
		}
	}

	// Recognizers didn't find anything, there's no trait, let's cache that.
	free(data);
	trait_cache_insert(&self->cache, &(trait_cache_Entry){
		.key = type,
		.val = {
			.owned = false,
			.data = nullptr,
		},
	});

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
