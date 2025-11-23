#include "vec.h"
#include "allocator.h"

void internal_vec_free(erased_vec* arr) {
	if (arr->data != nullptr) nil_free(arr->data);
	memset(arr, 0, sizeof(erased_vec));
}

void* internal_vec_reserve_item(erased_vec* arr, const usize item_size) {
	arr->len++;

	if (arr->cap < arr->len) {
		internal_vec_reallocate(arr, item_size, arr->len * 2);
	}

	return arr->data + (arr->len - 1) * item_size;
}

void internal_vec_reallocate(erased_vec* arr, const usize item_size, const usize new_cap) {
	arr->data = nil_realloc(arr->data, new_cap * item_size);
	/*void* new_data = nil_alloc(new_cap * item_size);

	if (arr->data != nullptr) {
		memcpy(new_data, arr->data, arr->len * item_size);

		nil_free(arr->data);
	}*/

	arr->cap = new_cap;
	// arr->data = new_data;
}

void internal_vec_pop(erased_vec* arr, usize item_size, void* dst) {
	arr->len--;

	if (dst != nullptr)
		memcpy(dst, arr->data + arr->len * item_size, item_size);

	memset(arr->data + arr->len * item_size, 0, item_size);
}

void internal_vec_copy_from(erased_vec* arr, const usize item_size, const void* src, const usize count) {
	if (arr->cap < count) {
		void* newData = nil_alloc(count * item_size);

		if (arr->data != nullptr) {
			nil_free(arr->data);
		}

		arr->cap = count;
		arr->data = newData;
	}

	memcpy(arr->data, src, count * item_size);
	arr->len = count;
}

void internal_vec_remove(erased_vec* vec, const usize item_size, const usize index) {
	if (index < vec->len - 1)
		memmove(
			vec->data + item_size * index,
			vec->data + item_size * (index+1),
			(vec->len - index - 1) * item_size
		);
	vec->len--;
}

void internal_vec_remove_swap_last(erased_vec* vec, const usize item_size, const usize index) {
	if (vec->len > 1 && index < vec->len - 1)
		memcpy(vec->data + item_size * index, vec->data + item_size * (vec->len - 1), item_size);
	vec->len--;
}

/*bool internal_vec_contains(erased_vec* arr, usize item_size, void* value) {
	return arrayContains(arr->data, arr->len, item_size, value);
}*/


void* internal_slotted_vec_reserve_slot(erased_vec* arr, const usize slotSize, const usize generationOffset) {
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

	void* slot = internal_vec_reserve_item(arr, slotSize);

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
