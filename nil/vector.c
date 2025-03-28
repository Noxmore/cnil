#include <malloc.h>
#include <string.h>
#include "vector.h"
#include "stdio.h"

typedef struct InternalVectorInfo {
	// In this day and age, we can spare the extra bytes to reduce bugs.
	usize magic;
	usize cap;
	usize len;
} InternalVectorInfo;

static void printInvalidMagicNumberErr() {
#ifndef N_VECTOR_SUPPRESS_ERRORS
	fprintf(stderr, "Invalid magic number for vector, probably not a vector");
#endif
}

static InternalVectorInfo* internalInfo(void* arr) {
	if (arr == nullptr) return nullptr;
	InternalVectorInfo* info = arr - sizeof(InternalVectorInfo);
	if (info->magic != N_VECTOR_MAGIC_NUMBER) {
		printInvalidMagicNumberErr();
		return nullptr;
	}

	return info;
}

usize vecLen(vector vec) {
	auto info = internalInfo(vec);
	if (info == nullptr) return 0;
	return info->len;
}
usize vecCap(vector vec) {
	auto info = internalInfo(vec);
	if (info == nullptr) return 0;
	return info->cap;
}
void vecDrop(vector vec) {
	auto info = internalInfo(vec);
	if (info == nullptr) return;
	free(info);
}

void* internal_vecReserveItem(vector* vec, usize itemSize) {
	if (vec == nullptr) return nullptr;
	auto info = internalInfo(*vec);
	if (info == nullptr) return nullptr;

	info->len++;

	if (info->cap < info->len) {
		internal_vecReallocate(vec, itemSize, info->len * 2);
	}

	return *vec + (info->len - 1) * itemSize;
}

void internal_vecReallocate(vector* vec, usize itemSize, usize newCap) {
	if (vec == nullptr) return;
	auto info = internalInfo(*vec);

	// void* newData = malloc(newCap * itemSize);

	if (info == nullptr) {
		info = malloc(sizeof(InternalVectorInfo) + newCap * itemSize);
		info->magic = N_VECTOR_MAGIC_NUMBER;
		info->len = 0;
		*vec = info + sizeof(InternalVectorInfo);
	} else {
		*vec = realloc(info, sizeof(InternalVectorInfo) + newCap * itemSize);

		// memcpy(newData, arr->data, arr->len * itemSize);
		// free(arr->data);

	}

	info->cap = newCap;
}

void internal_vecPop(vector vec, usize itemSize, void* dst) {
	auto info = internalInfo(vec);
	if (info == nullptr) return;

	info->len--;

	if (dst != nullptr)
		memcpy(dst, vec + info->len * itemSize, itemSize);

	// memset(vec + info->len * itemSize, 0, itemSize);
}

void internal_vecCopyFrom(vector* vec, usize itemSize, const void* src, usize count) {
	if (vec == nullptr) return;
	auto info = internalInfo(*vec);
	if (info == nullptr) return;

	if (info->cap < count) {
		void* newData = malloc(count * itemSize);

		if (*vec != nullptr) {
			free(*vec);
		}

		info->cap = count;
		*vec = newData;
	}

	memcpy(*vec, src, count * itemSize);
	info->len = count;
}

/*bool internal_vecContains(vector vec, usize itemSize, void* value) {
	return arrayContains(arr->data, arr->len, itemSize, value);
}*/