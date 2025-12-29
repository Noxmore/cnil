#pragma once

#include "nil/reflect.h"
#include "nil/vec.h"

typedef struct ANNOTATE("type_annotation") complex_type {
	int foo;
	char fixed_str[10];
	int array_2d[8][8];
	vec(u32) vector;

	struct {
		int x;
		int y;
	} anon_type;
} complex_type;
REFLECT_TYPE(complex_type)

void add_reflection_tests();