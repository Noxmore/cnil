#include "vec.h"
#include <malloc.h>

void internal_vecDrop(ErasedVec* arr) {
	if (arr->data != nullptr) free(arr->data);
	memset(arr, 0, sizeof(ErasedVec));
}

void* internal_vecReserveItem(ErasedVec* arr, usize itemSize) {
	arr->len++;

	if (arr->cap < arr->len) {
		internal_vecReallocate(arr, itemSize, arr->len * 2);
	}

	return arr->data + (arr->len - 1) * itemSize;
}

void internal_vecReallocate(ErasedVec* arr, usize itemSize, usize newCap) {
	void* newData = malloc(newCap * itemSize);

	if (arr->data != nullptr) {
		memcpy(newData, arr->data, arr->len * itemSize);

		free(arr->data);
	}

	arr->cap = newCap;
	arr->data = newData;
}

void internal_vecPop(ErasedVec* arr, usize itemSize, void* dst) {
	arr->len--;

	if (dst != nullptr)
		memcpy(dst, arr->data + arr->len * itemSize, itemSize);

	memset(arr->data + arr->len * itemSize, 0, itemSize);
}

void internal_vecCopyFrom(ErasedVec* arr, usize itemSize, void* src, usize count) {
	if (arr->cap < count) {
		void* newData = malloc(count * itemSize);

		if (arr->data != nullptr) {
			free(arr->data);
		}

		arr->cap = count;
		arr->data = newData;
	}

	memcpy(arr->data, src, count * itemSize);
	arr->len = count;
}

/*bool internal_vecContains(ErasedVec* arr, usize itemSize, void* value) {
	return arrayContains(arr->data, arr->len, itemSize, value);
}*/


string strNew(const char* str) {
	usize len = strlen(str);
	usize cap = len + 1; // Account for the null-terminator.

	char* data = malloc(cap);
	memcpy(data, str, cap);

	return (string){
		.data = data,
		.len = len,
		.cap = cap,
	};
}

void strDrop(string* str) {
	free(str->data);
	*str = (string){0};
}

void* internal_slottedVecReserveSlot(ErasedVec* arr, usize slotSize, usize generationOffset) {
//	usize slotSize = sizeof(bool) + slotSize + sizeof(usize);
	for (usize i = 0; i < arr->len; i++) {
		void* slot = arr->data + i * slotSize;
		// This is why it must be the first field.
		bool* present = (bool*)slot;
		usize* generation = (usize*)(slot + generationOffset);

		if (!*present) {
			// Increase the generation
//			(*(usize*)(slot + slotSize - sizeof(usize)))++;
			memset(slot, 0, slotSize);

			(*present) = true;
			(*generation)++;
			return slot;
		}
	}

	void* slot = internal_vecReserveItem(arr, slotSize);

	bool* present = (bool*)slot;

	memset(slot, 0, slotSize);

	(*present) = true;

	return slot;
}

/*
usize internal_slottedDynArrayAdd(erased_dyn_array* arr, usize slotSize, const void* item) {
	usize index = internal_slottedDynArrayReserveSlot(arr, slotSize);

	memcpy(arr->data + index * slotSize + sizeof(bool), item, slotSize - sizeof(usize) - sizeof(bool));

	return index;
}*/
