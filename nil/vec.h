#pragma once

#include "nint.h"
#include <string.h>
#include <stddef.h>

// Heap-allocated growable array.
#define Vec(T) struct Vec$##T { T* data; usize len; usize cap; }
// Anonymous version of `Vec`
#define VecAnon(T) struct Vec { T* data; usize len; usize cap; }
#define vecDrop($arr) internal_vecDrop((ErasedVec*)($arr))
// Reserves a new item onto the vector and returns the pointer to it.
#define vecReserveItem($arr) (typeof(($arr)->data))internal_vecReserveItem((ErasedVec*)($arr), sizeof(*($arr)->data))
#define vecPush($arr, ...) *vecReserveItem($arr) = (__VA_ARGS__)
#define vecPop($arr, $dst) internal_vecPop((ErasedVec*)($arr), sizeof(*($arr)->data), ($dst))
// Reserve this many more elements.
#define vecReserve($arr, $cap) internal_vecReallocate((ErasedVec*)($arr), sizeof(*($arr)->data), ($arr)->cap + ($cap))
#define vecCopyFrom($arr, $src, $count) internal_vecCopyFrom((ErasedVec*)($arr), sizeof(*($arr)->data), $src, $count)
// Sets $dst to true if any element of $arr passes $predicate. use the `elem` ident in $predicate for the current element.
#define vecShortCircuit($arr, $dst, $predicate) do {\
	auto arr = $arr;\
	auto dst = $dst;\
	for (usize i = 0; i < arr->len; i++) {\
		auto elem = arr->data[i];\
		if ($predicate) *dst = true;\
	}\
} while (false)
// #define vecContains($arr, $value) internal_vecContains((ErasedVec*)($arr), sizeof(*($arr)->data), ($value))
#define vecClear(vec) (vec)->len = 0

typedef struct ErasedVec {
	void* data;
	size_t len;
	size_t cap;
} ErasedVec;

void internal_vecDrop(ErasedVec* arr);
void* internal_vecReserveItem(ErasedVec* arr, usize itemSize);
void internal_vecReallocate(ErasedVec* arr, usize itemSize, usize newCap);
void internal_vecPop(ErasedVec* arr, usize itemSize, void* dst);
void internal_vecCopyFrom(ErasedVec* arr, usize itemSize, void* src, usize count);
bool internal_vecContains(ErasedVec* arr, usize itemSize, void* value);

// Heap-allocated growable string type. Interface with like a vector.
typedef struct string {
	/// A null terminated c string.
	char* data;
	usize len;
	usize cap;
} string;

// Creates a string via a null terminated C string.
string strNew(const char* str);
void strDrop(string* str);

/*static inline void stringPrintDebug(string* str) {
	printf("data: %s, len: %lu, cap: %lu\n", str->data, str->len, str->cap);
}*/

void* internal_slottedVecReserveSlot(ErasedVec* arr, usize slotSize, usize generationOffset);
//usize internal_slottedvecAdd(ErasedVec* arr, usize slotSize, const void* item);

// `present` MUST be the first element in each slot.

#define SlottedVec(T) struct SlottedVec$##T { struct T##Slot { bool present; T value; usize generation; }* slots; usize len; usize cap; }
// Returns a pointer to the slot added.
#define slottedVecAdd($arr) (typeof(($arr)->slots))internal_slottedVecReserveSlot((ErasedVec*)($arr), sizeof(*($arr)->slots), offsetof(typeof(*($arr)->slots), generation))

#define slottedVecGetFromId($arr, $id) \
	if (($id).index >= ($arr).len) return nullptr; \
	auto slot = &($arr).slots[($id).index]; \
	if (!slot->present || slot->generation != ($id).generation) return nullptr
