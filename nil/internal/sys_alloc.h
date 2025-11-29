#pragma once

#include "../nint.h"

usize nil_page_size();
void* nil_reserve_page();
void nil_free_page(void* page_ptr);