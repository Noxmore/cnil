#pragma once

#include "alloc.h"
#include "macro_utils.h"
#include "reflect.h"

constexpr usize TRAIT_REGISTRY_PTR_WORDS = 12;

// Currently doesn't support multithreading. If you try, you will get weird results or crash!
typedef struct trait_registry {
	usize private[TRAIT_REGISTRY_PTR_WORDS];
} trait_registry;

// A function that automatically implements a trait for a type, writing into `data`. Returns false if the type is ineligible for this recognizer.
typedef bool (*trait_recognizer)(const type_info* type, void* data);

void trait_registry_init(trait_registry* registry, usize size, usize align, nil_free_fn free);
void trait_registry_free(trait_registry* registry);

void trait_registry_impl_recognizer(trait_registry* registry, trait_recognizer recognizer);

// Implements a trait for a type, copying the data behind `trait_data`, taking ownership of the object.
void trait_registry_impl_owned(trait_registry* registry, const type_info* type, const void* trait_data);
// Implements a trait for a type but uses the supplied data pointer directly instead of allocating and copying.
// Use this when statically implementing traits.
void trait_registry_impl_static(trait_registry* registry, const type_info* type, void* trait_data);

// Attempts to get the registered trait for `type` from `registry`. Returns `nullptr` if not implemented.
// If the trait is cached, returns it. If not, attempts to run recognizers to implement it, caching the result.
const void* trait_registry_get(trait_registry* registry, const type_info* type);

// Fun/diagnostic function to get the total amount of times a trait has been implemented.
usize total_implemented_traits();

#define NIL_TRAIT_REGISTRY_NAME(TRAIT) TRAIT##_registry

#define DECLARE_TRAIT(TRAIT) \
	static_assert(sizeof(TRAIT)|1); \
	extern trait_registry NIL_TRAIT_REGISTRY_NAME(TRAIT);

#define NIL_TRAIT_INIT_CONSTRUCTOR_PRIORITY 101
#define NIL_TRAIT_IMPL_CONSTRUCTOR_PRIORITY 102

// Implementation. Do not use in header files.
#define DEFINE_TRAIT(TRAIT, FREE) \
	trait_registry NIL_TRAIT_REGISTRY_NAME(TRAIT); \
	__attribute__((constructor(NIL_TRAIT_INIT_CONSTRUCTOR_PRIORITY))) static void NIL_CONCAT_2(NIL_TRAIT_REGISTRY_NAME(TRAIT), _auto_init)() { \
		trait_registry_init(&NIL_TRAIT_REGISTRY_NAME(TRAIT), sizeof(TRAIT), alignof(TRAIT), FREE); \
	} \
	__attribute__((destructor(NIL_TRAIT_INIT_CONSTRUCTOR_PRIORITY))) static void NIL_CONCAT_2(NIL_TRAIT_REGISTRY_NAME(TRAIT), _auto_free)() { \
		trait_registry_free(&NIL_TRAIT_REGISTRY_NAME(TRAIT)); \
	}

// Implementation. Do not use in header files.
#define IMPL_TRAIT(TRAIT, T, ...) \
	static TRAIT TRAIT##_impl_##T##__ = __VA_ARGS__; \
	__attribute__((constructor(NIL_TRAIT_IMPL_CONSTRUCTOR_PRIORITY))) static void NIL_GENERATED_COUNTER_NAME(TRAIT##_impl)() { \
		trait_registry_impl_static(&NIL_TRAIT_REGISTRY_NAME(TRAIT), TYPE_INFO(T), &TRAIT##_impl_##T##__); \
	}

// Implementation. Do not use in header files.
#define IMPL_TRAIT_RECOGNIZER(TRAIT, RECOGNIZER) \
	static_assert(sizeof(TRAIT)|1); \
	__attribute__((constructor(NIL_TRAIT_IMPL_CONSTRUCTOR_PRIORITY))) static void NIL_GENERATED_COUNTER_NAME(TRAIT##_add_recognizer)() { \
		trait_registry_impl_recognizer(&NIL_TRAIT_REGISTRY_NAME(TRAIT), RECOGNIZER); \
	}

#define trait_get(TRAIT, TYPE_INFO) (const TRAIT*)trait_registry_get(&NIL_TRAIT_REGISTRY_NAME(TRAIT), (TYPE_INFO))



// -- COMMON TRAITS -- //

typedef struct destructor_trait {
	nil_free_fn free;
} destructor_trait;
DECLARE_TRAIT(destructor_trait)
#define IMPL_FREE(T, FN) \
	static_assert(sizeof(T)|1); \
	static void erased_free_fn_##T(void* data) { FN((T*)data); } \
	IMPL_TRAIT(destructor_trait, T, { .free = erased_free_fn_##T })

typedef struct default_trait {
	void (*set_default)(void* self);
} default_trait;
DECLARE_TRAIT(default_trait)
#define IMPL_ZEROED_DEFAULT(T) \
	static void zero_out_default_##T(void* self) { *(T*)self = (T){0}; } \
	IMPL_TRAIT(default_trait, T, { .set_default = zero_out_default_##T })

typedef struct clone_trait {
	// Can be nullptr, meaning this object is not cloneable.
	void (*clone_into)(const void* self, void* other);
} clone_trait;
DECLARE_TRAIT(clone_trait)

typedef struct type_indirection_trait {
	const type_info* (*type)(const void* self, const type_registry* registry);
	void* (*data)(const void* self, const type_registry* registry);
} type_indirection_trait;

// `type_info` conversion functions. Mainly used for opaque/primitive types.
// Any and all of these can be null, which signals "unimplemented" or "unsupported".
typedef struct primitive_conversion_trait {
	// Write as a string directly to a file to avoid having to allocate via `to_string` if not needed.
	void (*write_string)(const void* self, FILE* file);
	string (*to_string)(const void* self, allocator_ref alloc);
	double (*to_floating)(const void* self);
	s64 (*to_integer)(const void* self);
	bool (*from_string)(void* self, str s, allocator_ref alloc);
	bool (*from_floating)(void* self, double v, allocator_ref alloc);
	bool (*from_integer)(void* self, s64 v, allocator_ref alloc);
} primitive_conversion_trait;
DECLARE_TRAIT(primitive_conversion_trait)

typedef struct dynamic_iterator {
	void* data;
	// Extra data stored on the stack to avoid having to allocate for the iterator in many cases.
	u64 stack_data[2];

	bool (*next)(struct dynamic_iterator* iter, void** dst);
	void (*free)(struct dynamic_iterator* iter);
} dynamic_iterator;

typedef struct list_trait {
	const type_info* element_type;

	usize (*len)(const struct list_trait* trait, const void* self);

	void (*reserve)(const struct list_trait* trait, void* self, usize elements, allocator_ref alloc);
	void* (*push_new)(const struct list_trait* trait, void* self, allocator_ref alloc);
	// void (*insert)(const struct list_trait* trait, void* self, usize index, const void* element);
	void (*remove)(const struct list_trait* trait, void* self, usize index);

	dynamic_iterator (*iter)(const struct list_trait* trait, void* self);
	dynamic_iterator (*const_iter)(const struct list_trait* trait, const void* self);
} list_trait;
DECLARE_TRAIT(list_trait)
