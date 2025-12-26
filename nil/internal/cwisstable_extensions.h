#pragma once

#include "cwisstable.h"

// TODO: Eventually, I would like to roll my own hash tables, at least have a good frontend for cwisstable.

#define NIL_CWISS_IDENTITY_POLICY_HASH_FUNCTION(NAME, K) \
	static inline size_t k_##NAME##_policy_hash(const void* val) { \
		K key; \
		memcpy(&key, val, sizeof(K)); \
		return (size_t)key; \
	}

#define CWISS_DECLARE_IDENTITY_KEYED_HASHMAP(NAME, K, V) \
	NIL_CWISS_IDENTITY_POLICY_HASH_FUNCTION(NAME, K) \
	CWISS_DECLARE_NODE_MAP_POLICY(k_##NAME##_policy, K, V, (key_hash, k_##NAME##_policy_hash)); \
	CWISS_DECLARE_HASHMAP_WITH(NAME, K, V, k_##NAME##_policy);

static size_t cwiss_set_identity_policy_hash(const void* val) {
	return *(usize*)val;
}
CWISS_DECLARE_NODE_SET_POLICY(identity_set_policy, usize, (key_hash, cwiss_set_identity_policy_hash));
CWISS_DECLARE_HASHSET_WITH(generic_hash_set, usize, identity_set_policy);