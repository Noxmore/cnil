#pragma once

#include "nint.h"
#include "alloc.h"
#include "type_layout.h"

// TODO: This could be another file.
#include "string.h"

#define NIL_HASHTABLE_GROUP_SIZE 16

typedef struct nil_hashtable_policy {
	u64 (*hash)(const void* key, usize key_size);
	bool (*eq)(const void* key_a, const void* key_b, usize key_size);
	// Can be `nullptr`.
	void (*free)(void* entry);
} nil_hashtable_policy;

// Default hashtable policy for plain-old-data.
extern const nil_hashtable_policy nil_pod_hashtable_policy;

// Doesn't hash keys, passes the bytes right through.
extern const nil_hashtable_policy nil_passthrough_hashtable_policy;

extern const nil_hashtable_policy nil_str_hashtable_policy;
extern const nil_hashtable_policy nil_string_hashtable_policy;

// ============================================================================================================================== //
//                                                              SHARED                                                            //
// ============================================================================================================================== //

typedef struct erased_hashtable {
	// nil_hashtable_ctrl* ctrl;
	u8* ctrl;
	void* data;
	const nil_hashtable_policy* policy;
	u32 cap; // Must be a power of 2.
	u32 len;
	u32 dead;
} erased_hashtable;

#define hashtable_next(SELF, CURRENT) (const typeof(NIL_HASHTABLE_ENTRY(SELF))*)erased_hashtable_next((const erased_hashtable*)(SELF), sizeof(NIL_HASHTABLE_ENTRY(SELF)), (CURRENT))
#define hashtable_next_mut(SELF, CURRENT) (typeof(NIL_HASHTABLE_ENTRY(SELF))*)erased_hashtable_next_mut((erased_hashtable*)(SELF), sizeof(NIL_HASHTABLE_ENTRY(SELF)), (CURRENT))

#define hashtable_foreach(VARNAME, TABLE) for (auto VARNAME = hashtable_next(TABLE, nullptr); VARNAME; VARNAME = hashtable_next(TABLE, VARNAME))
#define hashtable_foreach_mut(VARNAME, TABLE) for (auto VARNAME = hashtable_next_mut(TABLE, nullptr); VARNAME; VARNAME = hashtable_next_mut(TABLE, VARNAME))

// The `key_size` is separate from `entry_layout` to allow only hashing the first part of the entry (the key).
void erased_hashtable_insert(erased_hashtable* self, allocator_ref allocator, const void* entry, type_layout entry_layout, usize key_size);
bool erased_hashtable_remove(erased_hashtable* self, const void* key, usize entry_size, usize key_size, void* entry_dest);

const void* erased_hashtable_get_entry(const erased_hashtable* self, const void* key, usize entry_size, usize key_size);
void* erased_hashtable_get_entry_mut(erased_hashtable* self, const void* key, usize entry_size, usize key_size);
bool erased_hashtable_contains(const erased_hashtable* self, const void* key, usize entry_size, usize key_size);

void erased_hashtable_clear(erased_hashtable* self, usize entry_size);
void erased_hashtable_free(erased_hashtable* self, allocator_ref allocator, type_layout entry_layout);

const void* erased_hashtable_next(const erased_hashtable* self, usize entry_size, const void* current);
void* erased_hashtable_next_mut(erased_hashtable* self, usize entry_size, void* current);

#define NIL_HASHTABLE_STRUCTURE(T) { \
	u8* ctrl; \
	T* data; \
	const nil_hashtable_policy* policy; \
	u32 cap; \
	u32 len; \
	u32 dead; \
}

#define NIL_HASHTABLE_ENTRY(SELF) (SELF)->data[0]

// ============================================================================================================================== //
//                                                             HASHSET                                                            //
// ============================================================================================================================== //

#define hashset_named(T, NAME) struct hashset_$_##NAME NIL_HASHTABLE_STRUCTURE(T)
#define hashset(T) hashset_named(T, T)
#define hashset_anon(T) struct NIL_HASHTABLE_STRUCTURE(T)

#define hashset_insert(SELF, ALLOCATOR, ...) do { typeof(NIL_HASHTABLE_ENTRY(SELF)) value__ = __VA_ARGS__; erased_hashtable_insert((erased_hashtable*)(SELF), (ALLOCATOR), &value__, layoutof(NIL_HASHTABLE_ENTRY(SELF)), sizeof(NIL_HASHTABLE_ENTRY(SELF))); } while (false)
#define hashset_remove_into(SELF, VALUE, DST) erased_hashtable_remove((erased_hashtable*)(SELF), (VALUE), sizeof(NIL_HASHTABLE_ENTRY(SELF)), sizeof(NIL_HASHTABLE_ENTRY(SELF)), (typeof(NIL_HASHTABLE_ENTRY(SELF))*)(DST))
#define hashset_remove(SELF, VALUE) hashset_remove_into(SELF, VALUE, nullptr)
#define hashset_get(SELF, VALUE) (const typeof(NIL_HASHTABLE_ENTRY(SELF))*)erased_hashtable_get_entry((const erased_hashtable*)(SELF), (VALUE), sizeof(NIL_HASHTABLE_ENTRY(SELF)), sizeof(NIL_HASHTABLE_ENTRY(SELF)))
#define hashset_get_mut(SELF, VALUE) (typeof(NIL_HASHTABLE_ENTRY(SELF))*)erased_hashtable_get_entry_mut((erased_hashtable*)(SELF), (VALUE), sizeof(NIL_HASHTABLE_ENTRY(SELF)), sizeof(NIL_HASHTABLE_ENTRY(SELF)))
#define hashset_contains(SELF, VALUE) erased_hashtable_contains((const erased_hashtable*)(SELF), (VALUE), sizeof(NIL_HASHTABLE_ENTRY(SELF)), sizeof(NIL_HASHTABLE_ENTRY(SELF)))
#define hashset_clear(SELF) erased_hashtable_clear((erased_hashtable*)(SELF), sizeof(NIL_HASHTABLE_ENTRY(SELF)))
#define hashset_free(SELF, ALLOCATOR) erased_hashtable_free((erased_hashtable*)(SELF), (ALLOCATOR), layoutof(NIL_HASHTABLE_ENTRY(SELF)))

// ============================================================================================================================== //
//                                                             HASHMAP                                                            //
// ============================================================================================================================== //

#define NIL_HASHMAP_KEY(SELF) NIL_HASHTABLE_ENTRY(SELF).key

#define hashtable_entry_t(TABLE) typeof(NIL_HASHTABLE_ENTRY(TABLE))

#define hashmap_named(K, V, NAME) struct hashmap_$_##NAME NIL_HASHTABLE_STRUCTURE(struct { K key; V value; })
#define hashmap(K, V) hashmap_named(K, V, K##_$_##V)
#define hashmap_anon(K, V) struct NIL_HASHTABLE_STRUCTURE(struct { K key; V value; })

#define hashmap_insert(SELF, ALLOCATOR, ...) do { typeof(NIL_HASHTABLE_ENTRY(SELF)) value__ = { __VA_ARGS__ }; erased_hashtable_insert((erased_hashtable*)(SELF), (ALLOCATOR), &value__, layoutof(NIL_HASHTABLE_ENTRY(SELF)), sizeof(NIL_HASHMAP_KEY(SELF))); } while (false)
#define hashmap_remove_into(SELF, KEY, DST) erased_hashtable_remove((erased_hashtable*)(SELF), (KEY), sizeof(NIL_HASHTABLE_ENTRY(SELF)), sizeof(NIL_HASHMAP_KEY(SELF)), (typeof(NIL_HASHTABLE_ENTRY(SELF))*)(DST))
#define hashmap_remove(SELF, KEY) hashmap_remove_into(SELF, KEY, nullptr)
#define hashmap_get(SELF, KEY) (&((const typeof(NIL_HASHTABLE_ENTRY(SELF))*)erased_hashtable_get_entry((const erased_hashtable*)(SELF), (KEY), sizeof(NIL_HASHTABLE_ENTRY(SELF)), sizeof(NIL_HASHMAP_KEY(SELF))))->value)
#define hashmap_get_mut(SELF, KEY) (&((typeof(NIL_HASHTABLE_ENTRY(SELF))*)erased_hashtable_get_entry_mut((erased_hashtable*)(SELF), (KEY), sizeof(NIL_HASHTABLE_ENTRY(SELF)), sizeof(NIL_HASHMAP_KEY(SELF))))->value)
#define hashmap_contains(SELF, KEY) erased_hashtable_contains((const erased_hashtable*)(SELF), (KEY), sizeof(NIL_HASHTABLE_ENTRY(SELF)), sizeof(NIL_HASHMAP_KEY(SELF)))
#define hashmap_clear(SELF) erased_hashtable_clear((erased_hashtable*)(SELF), sizeof(NIL_HASHTABLE_ENTRY(SELF)))
#define hashmap_free(SELF, ALLOCATOR) erased_hashtable_free((erased_hashtable*)(SELF), (ALLOCATOR), layoutof(NIL_HASHTABLE_ENTRY(SELF)))
