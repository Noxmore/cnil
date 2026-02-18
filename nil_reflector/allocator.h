#pragma once

#include "nil/alloc.h"

// Because of how simple this program is, we have a single static allocator stored here.
extern allocator_ref alloc;