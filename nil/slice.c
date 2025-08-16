#include "slice.h"

slice internal_subslice(const slice slice, const u32 from, const u32 to) {
	return (struct slice){
		.data = slice.data + from * slice.block_size,
		.len = to - from,
		.block_size = slice.block_size,
	};
}
