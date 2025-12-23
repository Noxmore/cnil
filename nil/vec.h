#pragma once

#include "nint.h"
// #include <string.h>
// #include <stddef.h>

// Heap-allocated growable array.
#define vec_named(T, NAME) struct vec_$_##NAME { T* data; usize len; usize cap; }
// Heap-allocated growable array.
#define vec(T) vec_named(T, T)
// Like `vec`, but doesn't specify a struct name.
// This can be useful when creating a vector of pointer types, but doesn't cover some cases that a named vector does. For those you should use `vec_named` or a typedef.
#define vec_anon(T) struct { T* data; usize len; usize cap; }
#define vec_free($arr) internal_vec_free((erased_vec*)($arr))
#define vec_free_with($arr, FREE_FN) do { for (usize i = 0; i < ($arr)->len; i++) { FREE_FN(&($arr)->data[i]); } vec_free($arr); } while (false)
// Reserves a new item onto the vector and returns the pointer to it.
#define vec_reserve_item($arr) (typeof(($arr)->data))internal_vec_reserve_item((erased_vec*)($arr), sizeof(*($arr)->data))
#define vec_push($arr, ...) *vec_reserve_item($arr) = (__VA_ARGS__)
#define vec_pop($arr, $dst) internal_vec_pop((erased_vec*)($arr), sizeof(*($arr)->data), ($dst))
// Reserve this many more elements.
#define vec_reserve($arr, $cap) internal_vec_reallocate((erased_vec*)($arr), sizeof(*($arr)->data), ($arr)->cap + ($cap))
#define vec_copy_from($arr, $src, $count) internal_vec_copy_from((erased_vec*)($arr), sizeof(*($arr)->data), $src, $count)
// Sets $dst to true if any element of $arr passes $predicate. Use the `elem` ident in $predicate for the current element.
#define vec_short_circuit($arr, $dst, $predicate) do {\
	auto arr = $arr;\
	for (usize i = 0; i < arr->len; i++) {\
		auto elem = arr->data[i];\
		if ($predicate) *($dst) = true;\
	}\
} while (false)
// #define vecContains($arr, $value) internal_vec_contains((erased_vec*)($arr), sizeof(*($arr)->data), ($value))
#define vec_clear(vec) (vec)->len = 0
#define vec_last(vec) (vec)->data[(vec)->len-1]
// Removes an element by moving all elements in front of it back a space. Preserves order.
#define vec_remove(VEC, IDX) internal_vec_remove((erased_vec*)(VEC), sizeof(*(VEC)->data), IDX)
// Removes an element in O(1) time by swapping the last element with it then subtracting 1 from length. Does not preserve order.
#define vec_remove_swap_last(VEC, IDX) internal_vec_remove_swap_last((erased_vec*)(VEC), sizeof(*(VEC)->data), IDX)
// #define vec_foreach($arr, $name) for (usize NIL_MACRO_VAR(i) = 0; NIL_MACRO_VAR(i) < ($arr)->len; NIL_MACRO_VAR(i)++)
// #define vec_for($arr, $i) for (usize $i = 0; $i < ($arr)->len; $i++)

/// Type-erased vector. Mainly used internally, but can also bee used for runtime-defined/runtime-sized types.
typedef struct erased_vec {
	void* data;
	usize len;
	usize cap;
} erased_vec;

void internal_vec_free(erased_vec* arr);
void* internal_vec_reserve_item(erased_vec* arr, usize item_size);
void internal_vec_reallocate(erased_vec* arr, usize item_size, usize new_cap);
void internal_vec_pop(erased_vec* arr, usize item_size, void* dst);
void internal_vec_copy_from(erased_vec* arr, usize item_size, const void* src, usize count);
bool internal_vec_contains(erased_vec* arr, usize item_size, void* value);
// Removes an element by moving all elements in front of it back a space. Preserves order.
void internal_vec_remove(erased_vec* vec, usize item_size, usize index);
// Removes an element in O(1) time by swapping the last element with it then subtracting 1 from length. Does not preserve order.
void internal_vec_remove_swap_last(erased_vec* vec, usize item_size, usize index);

/*static inline void stringPrintDebug(string* str) {
	printf("data: %s, len: %lu, cap: %lu\n", str->data, str->len, str->cap);
}*/

void* internal_slotted_vec_reserve_slot(erased_vec* arr, usize slotSize, usize generationOffset);
//usize internal_slottedvecAdd(erased_vec* arr, usize slotSize, const void* item);

// `present` MUST be the first element in each slot.

#define slotted_vec(T) struct slotted_vec_$_##T { struct T##Slot { bool present; T value; usize generation; }* slots; usize len; usize cap; }
// Returns a pointer to the slot added.
#define slotted_vec_add($arr) (typeof(($arr)->slots))internal_slotted_vec_reserve_slot((erased_vec*)($arr), sizeof(*($arr)->slots), offsetof(typeof(*($arr)->slots), generation))

#define slotted_vec_get_from_id($arr, $id) \
	if (($id).index >= ($arr).len) return nullptr; \
	auto slot = &($arr).slots[($id).index]; \
	if (!slot->present || slot->generation != ($id).generation) return nullptr
