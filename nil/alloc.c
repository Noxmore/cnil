#include "alloc.h"

#include "internal/sys_alloc.h"

// We need a block allocator for this

typedef struct arena_block {
	struct arena_block *next;
	u8 *buffer_end;
	u8 *head;
	u8 buffer[];
} arena_block;

/*typedef struct arena {
	arena_block *blocks;
} arena;*/



/*nil_allocator create_arena() {
	return (nil_allocator){

	};
}*/