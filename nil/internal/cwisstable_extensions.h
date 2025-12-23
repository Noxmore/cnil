#pragma once

#include "cwisstable.h"

#define CWISS_DECLARE_IDENTITY_KEYED_HASHMAP(NAME, K, V) \
	static inline size_t k_##NAME##_policy_hash(const void* val) { \
		K key; \
		memcpy(&key, val, sizeof(K)); \
		return (size_t)key; \
	} \
	CWISS_DECLARE_NODE_MAP_POLICY(k_##NAME##_policy, K, V, (key_hash, k_##NAME##_policy_hash)); \
	CWISS_DECLARE_HASHMAP_WITH(NAME, K, V, k_##NAME##_policy);
