#pragma once

#include "nint.h"
#include "macro_utils.h"

// Quick-and-easy hashing function. Prioritizes speed over security.
u64 nil_hash(const void* data, usize len);
// Shorthand for `nil_hash(&V, sizeof(V))`
#define nil_hash_single(V) nil_hash(&V, sizeof(V))

// `nil_hash` but with a specified seed.
u64 nil_hash_with_seed(usize seed, const void* data, usize len);
#define nil_hash_single_with_seed(SEED, V) nil_hash_with_seed(SEED, &V, sizeof(V))

u64 nil_internal_hash_all(usize pairs, ...);
// Call in pairs of `const void*, usize`, where each pair defines the start and length of an object to be hashed.
#define nil_hash_all(...) ((void)sizeof(struct { static_assert(NIL_COUNT_ARGS(__VA_ARGS__) % 2 == 0); }), nil_internal_hash_all(NIL_COUNT_ARGS(__VA_ARGS__)/2, __VA_ARGS__))

u64 nil_internal_hash_all_with_seed(usize seed, usize pairs, ...);
// Call variadic args in pairs of `const void*, usize`, where each pair defines the start and length of an object to be hashed.
#define nil_hash_all_with_seed(SEED, ...) ((void)sizeof(struct { static_assert(NIL_COUNT_ARGS(__VA_ARGS__) % 2 == 0); }), nil_internal_hash_all_with_seed(SEED, NIL_COUNT_ARGS(__VA_ARGS__)/2, __VA_ARGS__))

