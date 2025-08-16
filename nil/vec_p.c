#include "vec_p.h"

#include "allocator.h"
#include "math.h"

#include <memory.h>

/*static void* vec_at(erased_vec* vec, const usize index) {
	const vec_meta* info = vec_info(vec);
	if (info == nullptr || index >= info->len)
		return nullptr;
	return vec + info->len * info->type_size;
}*/

vec_meta* vec_info(erased_vec* vec) {
	if (vec == nullptr)
		return nullptr;
	return vec - sizeof(vec_meta);
}

usize vec_len(erased_vec* vec) {
	const vec_meta* info = vec_info(vec);
	return info == nullptr ? 0 : info->len;
}

usize vec_cap(erased_vec* vec) {
	const vec_meta* info = vec_info(vec);
	return info == nullptr ? 0 : info->cap;
}

void internal_vec_free(erased_vec** vec) {
	if (vec == nullptr || *vec == nullptr)
		return;
	nil_free(vec_info(*vec));
	*vec = nullptr;
}

void* internal_vec_reserve_item(erased_vec** vec, const usize type_size) {
	if (vec == nullptr)
		return nullptr;

	vec_meta* info = vec_info(*vec);
	usize len = 0;
	usize cap = 0;
	if (info != nullptr) {
		len = info->len;
		cap = info->cap;
	}

	// We need to grow
	if (len >= cap)
		internal_vec_reallocate(vec, max(len * 2, 2), type_size);

	info = vec_info(*vec);

	return *vec + info->len++ * info->type_size;
}

void vec_pop(erased_vec* vec, void* dst) {
	vec_meta* info = vec_info(vec);
	if (info == nullptr || info->len == 0)
		return;
	info->len--;
	memcpy(dst, vec + info->len * info->type_size, info->type_size);
}

void internal_vec_reallocate(erased_vec** vec, const usize cap, const usize type_size) {
	if (vec == nullptr) return;
	if (cap <= vec_cap(*vec)) return;

	erased_vec* old = *vec;
	const usize old_len = vec_len(old);

	vec_meta* new_info = nil_alloc(sizeof(vec_meta) + cap);
	new_info->type_size = type_size;
	new_info->len = old_len;
	new_info->cap = cap;

	*vec = (erased_vec*)new_info + sizeof(vec_meta);

	memset(*vec, 0, cap);
	memcpy(*vec, old, old_len);

	nil_free(old);
}
