#include "cunit.h"

#include "reflection.h"
#include "hashtable.h"

int main() {
	cunit_init();

	add_reflection_tests();
	add_hashtable_tests();

	return cunit_run();
}