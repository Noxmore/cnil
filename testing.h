#pragma once

#include "nil/reflect.h"

typedef struct reflected foo {
	int bar ANNOTATE("range(0..10)") ANNOTATE("default(5)");
	int* bar_pointer reflect_ignore;
	enum foo_type {
		foo_thing1,
		foo_thing2,
	} type;
} foo;
EXPORT_TYPE(foo)

enum reflected bar {
	bar_foo ANNOTATE("default"),
	bar_bar reflect_ignore,
	bar_baz,
};

union reflected baz {
	foo foo;
	enum bar bar;
};