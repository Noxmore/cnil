#pragma once

#include "internal/vec_types.h"
#include "type_layout.h"
#include "alloc.h"

#define vec_free(VEC, ALLOCATOR) erased_vec_free((erased_vec*)(VEC), (ALLOCATOR), layoutof(*(VEC)->data))
#define vec_free_with(VEC, ALLOCATOR, FREE_FN) do { for (usize i = 0; i < (VEC)->len; i++) { FREE_FN(&(VEC)->data[i]); } vec_free(VEC, ALLOCATOR); } while (false)
// Reserves a new item onto the vector and returns the pointer to it.
#define vec_reserve_item(VEC, ALLOCATOR) (typeof((VEC)->data))erased_vec_reserve_item((erased_vec*)(VEC), (ALLOCATOR), layoutof(*(VEC)->data))
#define vec_push(VEC, ALLOCATOR, ...) *vec_reserve_item(VEC, ALLOCATOR) = (__VA_ARGS__)
#define vec_pop_into(VEC, DST) erased_vec_pop((erased_vec*)(VEC), sizeof(*(VEC)->data), (DST))
#define vec_pop(VEC) vec_pop_into(VEC, nullptr)
// Reserve this many more elements.
#define vec_reserve(VEC, ALLOCATOR, CAP) erased_vec_reallocate((erased_vec*)(VEC), ALLOCATOR, layoutof(*(VEC)->data), (VEC)->cap + (CAP))
#define vec_copy_from(VEC, ALLOCATOR, SRC, COUNT) erased_vec_copy_from((erased_vec*)(VEC), (ALLOCATOR), layoutof(*(VEC)->data), SRC, COUNT)
// Sets $dst to true if any element of VEC passes $predicate. Use the `elem` ident in $predicate for the current element.
#define vec_short_circuit(VEC, DST, PREDICATE) do {\
	auto arr = VEC;\
	for (usize i = 0; i < arr->len; i++) {\
		auto elem = arr->data[i];\
		if (PREDICATE) *(DST) = true;\
	}\
} while (false)
// #define vecContains(VEC, $value) erased_vec_contains((erased_vec*)(VEC), sizeof(*(VEC)->data), ($value))
#define vec_clear(VEC) (VEC)->len = 0
#define vec_last(VEC) (VEC)->data[(VEC)->len-1]
// Removes an element by moving all elements in front of it back a space. Preserves order.
#define vec_remove(VEC, IDX) erased_vec_remove((erased_vec*)(VEC), sizeof(*(VEC)->data), IDX)
// Removes an element in O(1) time by swapping the last element with it then subtracting 1 from length. Does not preserve order.
#define vec_remove_swap_last(VEC, IDX) erased_vec_remove_swap_last((erased_vec*)(VEC), sizeof(*(VEC)->data), IDX)
// #define vec_foreach(VEC, $name) for (usize NIL_MACRO_VAR(i) = 0; NIL_MACRO_VAR(i) < (VEC)->len; NIL_MACRO_VAR(i)++)
// #define vec_for(VEC, $i) for (usize $i = 0; $i < (VEC)->len; $i++)

void erased_vec_free(erased_vec* vec, allocator_ref allocator, type_layout layout);
void* erased_vec_reserve_item(erased_vec* vec, allocator_ref allocator, type_layout layout);
void erased_vec_reallocate(erased_vec* vec, allocator_ref allocator, type_layout layout, usize new_cap);
void erased_vec_pop(erased_vec* vec, usize size, void* dst);
void erased_vec_copy_from(erased_vec* vec, allocator_ref allocator, type_layout layout, const void* src, usize count);
bool erased_vec_contains(erased_vec* vec, usize size, void* value);
// Removes an element by moving all elements in front of it back a space. Preserves order.
void erased_vec_remove(erased_vec* vec, usize size, usize index);
// Removes an element in O(1) time by swapping the last element with it then subtracting 1 from length. Does not preserve order.
void erased_vec_remove_swap_last(erased_vec* vec, usize size, usize index);

// void* internal_slotted_vec_reserve_slot(erased_vec* arr, usize slotSize, usize generationOffset);
//usize internal_slottedvecAdd(erased_vec* arr, usize slotSize, const void* item);

// `present` MUST be the first element in each slot.
