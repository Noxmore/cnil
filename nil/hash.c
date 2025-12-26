#include "hash.h"

// Currently, this just passes through to komihash, as it seems fine enough for now.
#include "internal/komihash.h"

#include <stdarg.h>

u64 nil_hash(const void* data, const usize len) {
	return komihash(data, len, 0);
}
u64 nil_hash_with_seed(const usize seed, const void* data, const usize len) {
	return komihash(data, len, seed);
}

u64 nil_internal_hash_all(usize pairs, ...) {
	va_list args;
	va_start(args, pairs);

	usize seed = 0;

	for (usize i = 0; i < pairs; i++) {
		const void* data = va_arg(args, const void*);
		usize len = va_arg(args, usize);

		if (len == 0 || data == nullptr)
			continue;

		seed = komihash(data, len, seed);
	}

	va_end(args);

	return seed;
}
u64 nil_internal_hash_all_with_seed(usize seed, usize pairs, ...) {
	va_list args;
	va_start(args, pairs);

	for (usize i = 0; i < pairs; i++) {
		const void* data = va_arg(args, const void*);
		usize len = va_arg(args, usize);

		if (len == 0 || data == nullptr)
			continue;

		seed = komihash(data, len, seed);
	}

	va_end(args);

	return seed;
}
