// The purpose of this file is to define a function that registers all reflected types that nil has.

#include "reflect.h"

#include "string.h"

#include <stdlib.h>

void nil_register_default_types(type_registry* reg) {
	type_register(reg, TYPE_INFO(u8));
	type_register(reg, TYPE_INFO(u16));
	type_register(reg, TYPE_INFO(u32));
	type_register(reg, TYPE_INFO(u64));
	type_register(reg, TYPE_INFO(u128));
	type_register(reg, TYPE_INFO(usize));
	type_register(reg, TYPE_INFO(uint));
	type_register(reg, TYPE_INFO(ushort));
	type_register(reg, TYPE_INFO(ulong));
	type_register(reg, TYPE_INFO(size_t));
	type_register(reg, TYPE_INFO(s8));
	type_register(reg, TYPE_INFO(s16));
	type_register(reg, TYPE_INFO(s32));
	type_register(reg, TYPE_INFO(s64));
	type_register(reg, TYPE_INFO(s128));
	type_register(reg, TYPE_INFO(int));
	type_register(reg, TYPE_INFO(short));
	type_register(reg, TYPE_INFO(long));
	type_register(reg, TYPE_INFO(float));
	type_register(reg, TYPE_INFO(double));

	type_register(reg, TYPE_INFO(string));
	type_register(reg, TYPE_INFO(str));
}