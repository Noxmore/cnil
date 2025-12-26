#pragma once

#include "reflect.h"
#include "macro_utils.h"

constexpr usize TRAIT_REGISTRY_PTR_WORDS = 12;

// Currently doesn't support multithreading. If you try, you will get weird results or crash!
#define trait_registry(T) struct T##_registry_t { T* private[TRAIT_REGISTRY_PTR_WORDS]; }
typedef struct trait_registry trait_registry;

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
#define NIL_REGISTRY_REF(TRAIT) ((trait_registry*)&NIL_REGISTRY_NAME(TRAIT))

#define DECLARE_TRAIT(TRAIT) extern trait_registry(TRAIT) NIL_REGISTRY_NAME(TRAIT);

// Implementation. Do not use in header files.
#define DEFINE_TRAIT(TRAIT, FREE) \
	trait_registry(TRAIT) NIL_REGISTRY_NAME(TRAIT); \
	__attribute__((constructor)) static void NIL_CONCAT_2(NIL_REGISTRY_NAME(TRAIT), _auto_init)() { \
		trait_registry_init(NIL_REGISTRY_REF(TRAIT), sizeof(TRAIT), alignof(TRAIT), FREE); \
	} \
	__attribute__((destructor)) static void NIL_CONCAT_2(NIL_REGISTRY_NAME(TRAIT), _auto_free)() { \
		trait_registry_free(NIL_REGISTRY_REF(TRAIT)); \
	}

// Implementation. Do not use in header files.
#define IMPL_TRAIT(TRAIT, T, VALUE) \
	static TRAIT NIL_MACRO_VAR(TRAIT##_impl) = VALUE; \
	__attribute__((constructor)) static void NIL_MACRO_VAR(TRAIT##_impl_fn)() { \
		trait_registry_impl_static(NIL_REGISTRY_REF(TRAIT), TYPE_INFO(T), &NIL_MACRO_VAR(TRAIT##_impl)); \
	}

#define trait_get(TRAIT, TYPE_INFO) (const TRAIT*)trait_registry_get(NIL_REGISTRY_REF(TRAIT), (TYPE_INFO))
