#include "vec.h"

#include "trait.h"

#include <stdlib.h>
#include <string.h>

#include "panic.h"

void internal_vec_free(erased_vec* arr) {
	if (arr->data != nullptr) free(arr->data);
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
	arr->data = realloc(arr->data, new_cap * item_size);
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
		void* newData = malloc(count * item_size);

		if (arr->data != nullptr) {
			free(arr->data);
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


// -- TRAIT IMPLS -- //

static bool is_type_vec(const type_info* type) {
	if (!type_info_contains_annotation(type, s(NIL_VEC_MARKER)))
		return false;

#ifndef NDEBUG
	if (
		type->size != sizeof(erased_vec) ||
		type->align != alignof(erased_vec) ||
		type->kind != type_info_struct ||
		type->struct_data.field_count != 3 ||
		!str_eq(type->struct_data.fields[0].name, s("data")) ||
		!type->struct_data.fields[0].is_pointer ||
		!str_eq(type->struct_data.fields[1].name, s("len")) ||
		type->struct_data.fields[1].field_type != TYPE_INFO(usize) ||
		!str_eq(type->struct_data.fields[2].name, s("cap")) ||
		type->struct_data.fields[2].field_type != TYPE_INFO(usize)
	) {
		panic("Invalid type marked as vector: %s", type->name.data);
	}
#endif

	return true;
}

static void zero_out_vec(void* self) {
	memset(self, 0, sizeof(erased_vec));
}

static bool recognize_default_for_vecs(const type_info* type, void* data) {
	default_trait* trait = data;

	if (!is_type_vec(type))
		return false;

	trait->set_default = zero_out_vec;
	return true;
}
IMPL_TRAIT_RECOGNIZER(default_trait, recognize_default_for_vecs)



static usize list_vec_len(const list_trait* trait, const void* self) {
	return ((erased_vec*)self)->len;

}
static void list_vec_reserve(const list_trait* trait, void* self, const usize elements) {
	internal_vec_reallocate(self, trait->element_type->size, ((erased_vec*)self)->cap + elements);
}
static void* list_vec_push_new(const list_trait* trait, void* self) {
	return internal_vec_reserve_item(self, trait->element_type->size);
}
/*static void list_vec_insert(const list_trait* trait, void* self, const usize index, const void* element) {

}*/
static void list_vec_remove(const list_trait* trait, void* self, const usize index) {
	internal_vec_remove(self, trait->element_type->size, index);
}

static bool list_vec_iter_next(dynamic_iterator* iter, void** dst) {
	const erased_vec* vec = iter->data;

	if (iter->stack_data[0] >= vec->len)
		return false;

	*dst = vec->data + iter->stack_data[0] * iter->stack_data[1];
	iter->stack_data[0]++;
	return true;
}
static dynamic_iterator list_vec_iter(const list_trait* trait, void* self) {
	static_assert(sizeof(usize) <= sizeof(void*));
	return (dynamic_iterator){
		.data = self,
		.stack_data = {0, trait->element_type->size},

		.next = list_vec_iter_next,
		.free = nullptr,
	};
}
static void list_vec_iter_free(dynamic_iterator* iter) {}
static dynamic_iterator list_vec_const_iter(const list_trait* trait, const void* self) {
	static_assert(sizeof(usize) <= sizeof(void*));
	return (dynamic_iterator){
		.data = (void*)self,
		.stack_data = {0, trait->element_type->size},

		.next = list_vec_iter_next,
		.free = list_vec_iter_free,
	};
}

static bool recognize_list_for_vecs(const type_info* type, void* data) {
	list_trait* trait = data;

	if (!is_type_vec(type))
		return false;

	*trait = (list_trait){
		.element_type = type->struct_data.fields[0].field_type,

		.len = list_vec_len,

		.reserve = list_vec_reserve,
		.push_new = list_vec_push_new,
		// .insert = list_vec_insert,
		.remove = list_vec_remove,

		.iter = list_vec_iter,
		.const_iter = list_vec_const_iter,
	};
	return true;
}
IMPL_TRAIT_RECOGNIZER(list_trait, recognize_list_for_vecs)