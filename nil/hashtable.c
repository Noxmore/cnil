#include "hashtable.h"

#include <assert.h>

#include "hash.h"

#include <string.h>
#include <emmintrin.h>  // SSE2 intrinsics

#include "math.h"
#include "reflect.h"

static bool pod_eq(const void* key_a, const void* key_b, usize key_size) {
	return memcmp(key_a, key_b, key_size) == 0;
}

const nil_hashtable_policy nil_pod_hashtable_policy = {
	.hash = nil_hash,
	.eq = pod_eq,
};

static u64 passthrough_hash(const void* key, usize key_size) {
	return nil_bytes_to_integer(key, key_size);
}

const nil_hashtable_policy nil_passthrough_hashtable_policy = {
	.hash = passthrough_hash,
	.eq = pod_eq,
};

static u64 policy_str_hash(const void* key, usize key_size) {
	const str s = *(const str*)key;
	return nil_hash(s.data, s.len);
}

static bool policy_str_eq(const void* key_a, const void* key_b, usize key_size) {
	return str_eq(*(const str*)key_a, *(const str*)key_b);
}

const nil_hashtable_policy nil_str_hashtable_policy = {
	.hash = policy_str_hash,
	.eq = policy_str_eq,
};

const nil_hashtable_policy nil_string_hashtable_policy = nil_str_hashtable_policy; // This probably works.



// Must be a power of 2.
constexpr usize STARTING_CAPACITY = NIL_HASHTABLE_GROUP_SIZE;


constexpr u8 CTRL_EMPTY = 0b11111111;
constexpr u8 CTRL_DELETED = 0b11111110;

// If ctrl metadata is empty or deleted.
static bool ctrl_is_open(u8 metadata) {
	return metadata >> 7;
}

static bool ctrl_is_full(u8 metadata) {
	return !ctrl_is_open(metadata);
}

static usize h1(u64 hash) {
	return (usize)hash;
}

static u8 h2(u64 hash) {
	return (u8)(hash >> 57);  // Top 7 bits
}

// Convert a bitmask to positions
static int trailing_zeros(u16 mask) {
	return __builtin_ctz(mask);
}

// ============================================================================================================================== //
//                                                           SIMD HELPERS                                                         //
// ============================================================================================================================== //

// Find matching H2 values in a group using SIMD
static u16 match_byte(__m128i group, u8 byte) {
	const __m128i target = _mm_set1_epi8((char)byte);
	const __m128i cmp = _mm_cmpeq_epi8(group, target);
	return (u16)_mm_movemask_epi8(cmp);
}

// Find empty slots in a group
static u16 match_empty(__m128i group) {
	return match_byte(group, CTRL_EMPTY);
}

// Helper for signed comparison (since we don't have _mm_cmpgt_epi8 for what we need)
static __m128i _mm_cmpgt_epi8_fixed(__m128i a, __m128i b) {
	// We want slots where ctrl < DELETED (i.e., EMPTY or DELETED)
	// EMPTY = -1, DELETED = -2, so we check if ctrl >= DELETED
	// This actually checks for high bit set (negative numbers)
	return _mm_cmpgt_epi8(b, a);
}

// Find empty or deleted slots in a group
static u16 match_empty_or_deleted(__m128i group) {
	const __m128i special = _mm_set1_epi8(CTRL_DELETED);
	const __m128i cmp = _mm_cmpgt_epi8_fixed(special, group);
	return (u16)_mm_movemask_epi8(cmp);
}

// ============================================================================================================================== //
//                                                              PROBING                                                           //
// ============================================================================================================================== //

typedef struct {
	usize mask; // capacity - 1 (for fast modulo via bitwise AND)
	usize offset; // Current position we're examining in the table
	usize index; // How many times we've probed
} probe_seq;

static probe_seq probe_start(u64 hash, usize mask) {
	return (probe_seq){
		.mask = mask,
		.offset = h1(hash & mask),
		.index = 0,
	};
}

static void probe_next(probe_seq* seq) {
	seq->index++;
	seq->offset = (seq->offset + seq->index) & seq->mask;
}

// ============================================================================================================================== //
//                                                           INTERNAL API                                                         //
// ============================================================================================================================== //

static usize table_allocated_size(const erased_hashtable* self, type_layout entry_layout) {
	return (self->cap * entry_layout.size) + (self->cap + NIL_HASHTABLE_GROUP_SIZE);
}

static void table_resize(erased_hashtable* self, allocator_ref allocator, type_layout entry_layout, usize key_size, usize new_cap) {
	assert(is_unsigned_power_of_two(new_cap));
	assert(new_cap <= U32_MAX);

	void* old_data = self->data;
	u8* old_ctrl = self->ctrl;
	const usize old_allocated_size = table_allocated_size(self, entry_layout);
	const usize old_cap = self->cap;

	const usize new_data_size = new_cap * entry_layout.size;
	const usize new_ctrl_size = new_cap + NIL_HASHTABLE_GROUP_SIZE;

	// We allocate a single buffer for both pointers. TODO: Perhaps we could remove one at some point?
	self->data = nil_alloc(allocator, nullptr, entry_layout.align, 0, new_data_size + new_ctrl_size);
	self->ctrl = self->data + new_data_size;
	self->cap = new_cap;
	self->len = 0;
	self->dead = 0;

	memset(self->ctrl, CTRL_EMPTY, new_ctrl_size);

	// Reinsert previous elements.
	for (usize i = 0; i < old_cap; i++) {
		if (ctrl_is_full(old_ctrl[i]))
			erased_hashtable_insert(self, allocator, old_data + i * entry_layout.size, entry_layout, key_size);
	}

	// Free old data.
	nil_alloc(allocator, old_data, entry_layout.align, old_allocated_size, 0);
}

// Retrieves a table's policy with a default fallback.
static const nil_hashtable_policy* table_policy(const erased_hashtable* self) {
	return self->policy != nullptr ? self->policy : &nil_pod_hashtable_policy;
}

// Returns -1 if the item could not be found.
static usize table_get_index(const erased_hashtable* self, const void* key, usize entry_size, usize key_size) {
	if (self->cap == 0)
		return -1;

	const auto policy = table_policy(self);

	const u64 hash = policy->hash(key, key_size);
	const u8 hash_h2 = h2(hash);
	const usize mask = self->cap - 1;

	probe_seq probe = probe_start(hash, mask);

	for (;;) {
		const usize offset = probe.offset;

		// Load a group of 16 control bytes
		const __m128i group = _mm_loadu_si128((__m128i*)&self->ctrl[offset]);

		u16 matches = match_byte(group, hash_h2);
		while (matches != 0) {
			const int pos = trailing_zeros(matches);
			const usize index = (offset + pos) & mask;

			const void* entry = self->data + index * entry_size;
			if (policy->eq(entry, key, key_size)) {
				return index;
			}

			// Clear this bit and continue to the next `1`
			matches &= matches - 1;
		}

		// If we hit an EMPTY slot, the key doesn't exist
		const u16 empties = match_empty(group);
		if (empties != 0) {
			return -1;
		}

		probe_next(&probe);
	}
}

static void table_remove_index(erased_hashtable* self, usize index, usize entry_size, void* entry_dest) {
	self->ctrl[index] = CTRL_DELETED;
	self->len--;
	self->dead++;

	// Update sentinel if needed
	if (index < NIL_HASHTABLE_GROUP_SIZE) {
		self->ctrl[self->cap + index] = CTRL_DELETED;
	}

	void* entry = self->data + index * entry_size;
	if (entry_dest != nullptr)
		memcpy(entry_dest, entry, entry_size);
	else {
		auto policy = table_policy(self);
		if (policy->free)
			policy->free(entry);
	}
}

// ============================================================================================================================== //
//                                                           EXTERNAL API                                                         //
// ============================================================================================================================== //

void erased_hashtable_insert(erased_hashtable* self, allocator_ref allocator, const void* entry, type_layout entry_layout, usize key_size) {
	if (self->cap == 0 || (self->len + self->dead) * 2 > self->cap) {
		table_resize(self, allocator, entry_layout, key_size, max_usize(self->cap * 2, STARTING_CAPACITY));
	}

	const auto policy = table_policy(self);

	const u64 hash = policy->hash(entry, key_size);
	const u8 hash_h2 = h2(hash);
	const usize mask = self->cap - 1;

	probe_seq probe = probe_start(hash, mask);

	for (;;) {
		const usize offset = probe.offset;

		// Load a group of 16 control bytes
		const __m128i group = _mm_loadu_si128((__m128i*)&self->ctrl[offset]);

		// Check for empty or deleted slots where we can insert
		const u16 empties = match_empty_or_deleted(group);
		if (empties != 0) {
			const int pos = trailing_zeros(empties);
			const size_t index = (offset + pos) & mask;

			// Check if this was DEAD...
			if (self->ctrl[index] == CTRL_DELETED) {
				self->dead--;
			}

			// Insert the element
			void* current_entry = self->data + entry_layout.size * index;
			if (ctrl_is_full(self->ctrl[index])) { // If we already have data here.
				// Destruct the old entry.
				if (policy->free)
					policy->free(current_entry);
			} else {
				self->len++;
			}
			self->ctrl[index] = hash_h2;
			memcpy(current_entry, entry, entry_layout.size);

			// Update sentinel (mirror first NIL_HASHTABLE_GROUP_SIZE bytes at the end)
			if (index < NIL_HASHTABLE_GROUP_SIZE) {
				self->ctrl[self->cap + index] = hash_h2;
			}

			return;
		}

		// No empty slots in this group, continue probing
		probe_next(&probe);
	}
}

bool erased_hashtable_remove(erased_hashtable* self, const void* key, usize entry_size, usize key_size, void* entry_dest) {
	const usize index = table_get_index(self, key, entry_size, key_size);

	if (index == -1)
		return false;

	table_remove_index(self, index, entry_size, entry_dest);

	return true;
}


const void* erased_hashtable_get_entry(const erased_hashtable* self, const void* key, usize entry_size, usize key_size) {
	const usize index = table_get_index(self, key, entry_size, key_size);
	if (index == -1) return nullptr;
	return self->data + index * entry_size;
}
void* erased_hashtable_get_entry_mut(erased_hashtable* self, const void* key, usize entry_size, usize key_size) {
	const usize index = table_get_index(self, key, entry_size, key_size);
	if (index == -1) return nullptr;
	return self->data + index * entry_size;
}
bool erased_hashtable_contains(const erased_hashtable* self, const void* key, usize entry_size, usize key_size) {
	return erased_hashtable_get_entry(self, key, entry_size, key_size) != nullptr;
}

void erased_hashtable_clear(erased_hashtable* self, usize entry_size) {
	for (usize i = 0; i < self->cap; i++) {
		if (ctrl_is_full(self->ctrl[i])) {
			table_remove_index(self, i, entry_size, nullptr);
		}
	}
}

void erased_hashtable_free(erased_hashtable* self, allocator_ref allocator, type_layout entry_layout) {
	if (self->data == nullptr)
		return;

	const auto policy = table_policy(self);

	if (policy->free != nullptr) for (usize i = 0; i < self->cap; i++) {
		if (ctrl_is_full(self->ctrl[i])) {
			policy->free(self->data + i * entry_layout.size);
		}
	}

	nil_alloc(allocator, self->data, entry_layout.align, table_allocated_size(self, entry_layout), 0);
}

// Iterators

static bool next_index(const erased_hashtable* self, usize entry_size, const void* current, usize* dst) {
	usize index;

	if (current == nullptr) {
		index = 0;
	} else {
		index = (current - self->data) / entry_size + 1;
	}

	if (index >= self->cap)
		return false;

	while (ctrl_is_open(self->ctrl[index])) {
		index++;

		if (index >= self->cap)
			return false;
	}

	*dst = index;
	return true;
}

const void* erased_hashtable_next(const erased_hashtable* self, usize entry_size, const void* current) {
	usize index;
	return next_index(self, entry_size, current, &index)
		? self->data + index * entry_size
		: nullptr;
}
void* erased_hashtable_next_mut(erased_hashtable* self, usize entry_size, void* current) {
	usize index;
	return next_index(self, entry_size, current, &index)
		? self->data + index * entry_size
		: nullptr;
}
