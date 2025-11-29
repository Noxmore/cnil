#include "sys_alloc.h"

#include <sys/mman.h>
#include <unistd.h>

usize nil_page_size() {
	return sysconf(_SC_PAGESIZE);
}
void* nil_reserve_page() {
	return mmap(nullptr, nil_page_size(), PROT_READ | PROT_WRITE, MAP_ANONYMOUS | MAP_PRIVATE, -1, 0);
}
void nil_free_page(void* page_ptr) {
	munmap(page_ptr, nil_page_size());
}