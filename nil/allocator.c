#include "allocator.h"

#include "panic.h"

#include <stdlib.h>
#include <memory.h>

// DEFAULT ALLOCATOR

static void* default_alloc(allocator* self, const usize byte_count, void* free_fn) {
	return malloc(byte_count);
}

static void* default_realloc(allocator* self, void* ptr, const usize byte_count) {
	return realloc(ptr, byte_count);
}

static void default_free(allocator* self, void* ptr_to_free) {
	free(ptr_to_free);
}

static void default_destroy(allocator* self) {}

// GLOBAL ALLOCATOR

allocator allocator_default() {
	return (allocator){
		.user_data = nullptr,
		.alloc = default_alloc,
		.realloc = default_realloc,
		.free = default_free,
		.destroy = default_destroy,
	};
}

_Thread_local allocator nil_global_allocator = {
	.user_data = nullptr,
	.alloc = default_alloc,
	.realloc = default_realloc,
	.free = default_free,
	.destroy = default_destroy,
};

void* nil_alloc(const usize byte_count) {
	return nil_global_allocator.alloc(&nil_global_allocator, byte_count, nullptr);
}

void* nil_realloc(void* ptr, usize byte_count) {
	return nil_global_allocator.realloc(&nil_global_allocator, ptr, byte_count);
}

void nil_free(void* ptr_to_free) {
	nil_global_allocator.free(&nil_global_allocator, ptr_to_free);
}

// STACK ALLOCATOR

/*typedef struct stack_allocator_header {
	const allocator* super;
	usize cap;
	usize head;
	void* buf;
} stack_allocator_header;

typedef struct stack_elem_header {
	allocator_free_fn free_fn;
	usize len;
} stack_elem_header;

// TODO:
static void* stack_alloc(allocator* self, const usize byte_count) {
	stack_allocator_header* header = self->user_data;

	panic("Ran out of space! Capacity: {int}", header->cap);

	return nullptr;
}

static void stack_free(allocator* self, void* ptr_to_free) {
	stack_elem_header* elem_header = ptr_to_free - sizeof(stack_elem_header);

	if (elem_header->free_fn != nullptr)
		elem_header->free_fn(ptr_to_free);

	memset(elem_header, 0, sizeof(stack_elem_header) + elem_header->len);
}

static void stack_destroy(allocator* self) {

}

allocator allocator_stack(const usize capacity) {
	void* data = nil_alloc(sizeof(stack_allocator_header) + capacity);
	stack_allocator_header* header = data;
	header->super = &nil_global_allocator;
	header->cap = capacity;
	header->head = 0;

	return (allocator){
		.user_data = data,
		.alloc = stack_alloc,
		.free = stack_free,
		.destroy = stack_destroy,
	};
}*/
