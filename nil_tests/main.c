#include "cunit.h"

#include "reflection.h"

int main() {
	cunit_init();

	add_reflection_tests();

	return cunit_run();
}