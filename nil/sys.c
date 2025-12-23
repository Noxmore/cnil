#pragma once

#ifdef __unix__
	#include "internal/sys_alloc_linux.c"
#elifdef _WIN32
	#include "internal/sys_alloc_windows.c"
#elifdef __APPLE__
	#include "internal/sys_alloc_mac.c"
#else
	#error "Unsupported operating system."
#endif