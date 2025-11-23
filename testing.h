#pragma once

#include "nil/reflect.h"

typedef struct foo {
	int bar ANNOTATE("range(0..10)") ANNOTATE("default(5)");
	int* bar_pointer REFLECT_IGNORE;
	enum foo_type {
		foo_thing1,
		foo_thing2,
	} type;
} foo_t;
REFLECT_TYPE(foo_t)

enum bar {
	bar_foo ANNOTATE("default"),
	bar_bar REFLECT_IGNORE,
	bar_baz,
};
REFLECT_TYPE(enum, bar)

union baz {
	foo_t foo;
	enum bar bar;
};
REFLECT_TYPE(union, baz)
