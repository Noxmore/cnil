#pragma once

#include "reflect.h"
#include "macro_utils.h"

constexpr usize TRAIT_REGISTRY_PTR_WORDS = 12;

// Currently doesn't support multithreading. If you try, you will get weird results or crash!
// struct erased_type_registry { usize private[TRAIT_REGISTRY_PTR_WORDS]; };
// #define trait_registry(T) struct T##_registry_t { T* private[TRAIT_REGISTRY_PTR_WORDS]; }
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

#define NIL_REGISTRY_NAME(TRAIT) TRAIT##_registry
// #define NIL_REGISTRY_REF(TRAIT) ((trait_registry*)&NIL_REGISTRY_NAME(TRAIT))

#define DECLARE_TRAIT(TRAIT) extern trait_registry NIL_REGISTRY_NAME(TRAIT);

#define NIL_TRAIT_INIT_CONSTRUCTOR_PRIORITY 101
#define NIL_TRAIT_IMPL_CONSTRUCTOR_PRIORITY 102

// Implementation. Do not use in header files.
#define DEFINE_TRAIT(TRAIT, FREE) \
	trait_registry NIL_REGISTRY_NAME(TRAIT); \
	__attribute__((constructor(NIL_TRAIT_INIT_CONSTRUCTOR_PRIORITY))) static void NIL_CONCAT_2(NIL_REGISTRY_NAME(TRAIT), _auto_init)() { \
		trait_registry_init(&NIL_REGISTRY_NAME(TRAIT), sizeof(TRAIT), alignof(TRAIT), FREE); \
	} \
	__attribute__((destructor(NIL_TRAIT_INIT_CONSTRUCTOR_PRIORITY))) static void NIL_CONCAT_2(NIL_REGISTRY_NAME(TRAIT), _auto_free)() { \
		trait_registry_free(&NIL_REGISTRY_NAME(TRAIT)); \
	}

// Implementation. Do not use in header files.
#define IMPL_TRAIT(TRAIT, T, ...) \
	static TRAIT NIL_MACRO_VAR(TRAIT##_impl) = __VA_ARGS__; \
	__attribute__((constructor(NIL_TRAIT_IMPL_CONSTRUCTOR_PRIORITY))) static void NIL_MACRO_VAR(TRAIT##_impl_fn)() { \
		trait_registry_impl_static(&NIL_REGISTRY_NAME(TRAIT), TYPE_INFO(T), &NIL_MACRO_VAR(TRAIT##_impl)); \
	}

#define trait_get(TRAIT, TYPE_INFO) (const TRAIT*)trait_registry_get(&NIL_REGISTRY_NAME(TRAIT), (TYPE_INFO))



// -- COMMON TRAITS -- //

typedef struct destructor_trait {
	nil_free_fn free;
} destructor_trait;
DECLARE_TRAIT(destructor_trait)
#define IMPL_FREE(T, FN) \
	static void erased_free_fn_##T(void* data) { FN((T*)data); } \
	IMPL_TRAIT(destructor_trait, T, { .free = erased_free_fn_##T })

// `type_info` conversion functions. Mainly used for opaque/primitive types.
// Any and all of these can be null, which signals "unimplemented" or "unsupported".
typedef struct primitive_conversion_trait {
	// TODO: Improve this function, make it not need to rely on files. Perhaps a stream API could help?
	void (*to_string)(const void* self, FILE* file);
	double (*to_floating)(const void* self);
	s64 (*to_integer)(const void* self);
	bool (*from_string)(void* self, str s);
	bool (*from_floating)(void* self, double v);
	bool (*from_integer)(void* self, s64 v);
} primitive_conversion_trait;

DECLARE_TRAIT(primitive_conversion_trait)