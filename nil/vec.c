#include "vec.h"

#include "trait.h"
#include "panic.h"

#include <string.h>

void erased_vec_free(erased_vec* vec, allocator_ref allocator, const type_layout layout) {
	if (vec->data != nullptr) allocator.alloc(allocator.ctx, vec->data, layout.align, layout.size * vec->cap, 0);
	memset(vec, 0, sizeof(erased_vec));
}

void* erased_vec_reserve_item(erased_vec* vec, allocator_ref allocator, const type_layout layout) {
	vec->len++;

	if (vec->cap < vec->len) {
		erased_vec_reallocate(vec, allocator, layout, vec->len * 2);
	}

	return vec->data + (vec->len - 1) * layout.size;
}

void erased_vec_reallocate(erased_vec* vec, allocator_ref allocator, const type_layout layout, const usize new_cap) {
	vec->data = allocator.alloc(allocator.ctx, vec->data, layout.align, layout.size * vec->cap, layout.size * new_cap);
	vec->cap = vec->data == nullptr ? 0 : new_cap;
}

void erased_vec_pop(erased_vec* vec, usize item_size, void* dst) {
	vec->len--;

	if (dst != nullptr)
		memcpy(dst, vec->data + vec->len * item_size, item_size);

	memset(vec->data + vec->len * item_size, 0, item_size);
}

void erased_vec_copy_from(erased_vec* vec, allocator_ref allocator, const type_layout layout, const void* src, const usize count) {
	erased_vec_reallocate(vec, allocator, layout, count);
	memcpy(vec->data, src, layout.size * count);
	vec->len = count;
}

void erased_vec_remove(erased_vec* vec, const usize item_size, const usize index) {
	if (index < vec->len - 1)
		memmove(
			vec->data + item_size * index,
			vec->data + item_size * (index+1),
			(vec->len - index - 1) * item_size
		);
	vec->len--;
}

void erased_vec_remove_swap_last(erased_vec* vec, const usize item_size, const usize index) {
	if (vec->len > 1 && index < vec->len - 1)
		memcpy(vec->data + item_size * index, vec->data + item_size * (vec->len - 1), item_size);
	vec->len--;
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
		type->struct_data.fields[0].pointer_layers != 1 ||
		!str_eq(type->struct_data.fields[1].name, s("len")) ||
		type->struct_data.fields[1].type != TYPE_INFO(usize) ||
		!str_eq(type->struct_data.fields[2].name, s("cap")) ||
		type->struct_data.fields[2].type != TYPE_INFO(usize)
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
static void list_vec_reserve(const list_trait* trait, void* self, const usize elements, allocator_ref allocator) {
	erased_vec_reallocate(self, allocator, (type_layout){ .size = trait->element_type->size, .align = trait->element_type->align }, ((erased_vec*)self)->cap + elements);
}
static void* list_vec_push_new(const list_trait* trait, void* self, allocator_ref allocator) {
	return erased_vec_reserve_item(self, allocator, (type_layout){ .size = trait->element_type->size, .align = trait->element_type->align });
}
/*static void list_vec_insert(const list_trait* trait, void* self, const usize index, const void* element) {

}*/
static void list_vec_remove(const list_trait* trait, void* self, const usize index) {
	erased_vec_remove(self, trait->element_type->size, index);
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
		.element_type = type->struct_data.fields[0].type,

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