#include "nil/reflect.h"
#include "/d/_dev/_lib/c/cnil/testing.h"

DEFINE_TYPE_INFO(foo_type) {
	.kind = type_info_enum,
	.name = "foo_type",
	.type_size = sizeof(enum foo_type),
	.type_align = alignof(enum foo_type),
	.annotations = nullptr,
	.annotation_count = 0,
	.free = nullptr,
	.enum_data.variant_count = 2,
	.enum_data.variants = (type_info_variant[]){
		{ .name = "foo_thing1", .value = foo_thing1, .annotations = nullptr, .annotation_count = 0, },
		{ .name = "foo_thing2", .value = foo_thing2, .annotations = nullptr, .annotation_count = 0, },
	},
};
DEFINE_TYPE_INFO(foo) {
	.kind = type_info_struct,
	.name = "foo",
	.type_size = sizeof(struct foo),
	.type_align = alignof(struct foo),
	.annotations = nullptr,
	.annotation_count = 0,
	.free = nullptr,
	.struct_data.field_count = 2,
	.struct_data.fields = (type_info_field[]){
		{ .name = "bar", .annotations = (const char*[]){"range(0..10)", "default(5)"}, .annotation_count = 2, .field_type = type_info_of(int), .struct_offset = __builtin_offsetof(struct foo, bar), .is_pointer = false, .is_const = false },
		{ .name = "type", .annotations = nullptr, .annotation_count = 0, .field_type = type_info_of(foo_type), .struct_offset = __builtin_offsetof(struct foo, type), .is_pointer = false, .is_const = false },
	},
};
DEFINE_TYPE_INFO(bar) {
	.kind = type_info_enum,
	.name = "bar",
	.type_size = sizeof(enum bar),
	.type_align = alignof(enum bar),
	.annotations = nullptr,
	.annotation_count = 0,
	.free = nullptr,
	.enum_data.variant_count = 2,
	.enum_data.variants = (type_info_variant[]){
		{ .name = "bar_foo", .value = bar_foo, .annotations = (const char*[]){"default"}, .annotation_count = 1, },
		{ .name = "bar_baz", .value = bar_baz, .annotations = nullptr, .annotation_count = 0, },
	},
};
DEFINE_TYPE_INFO(baz) {
	.kind = type_info_union,
	.name = "baz",
	.type_size = sizeof(union baz),
	.type_align = alignof(union baz),
	.annotations = nullptr,
	.annotation_count = 0,
	.free = nullptr,
	.struct_data.field_count = 2,
	.struct_data.fields = (type_info_field[]){
		{ .name = "foo", .annotations = nullptr, .annotation_count = 0, .field_type = type_info_of(foo), .struct_offset = __builtin_offsetof(union baz, foo), .is_pointer = false, .is_const = false },
		{ .name = "bar", .annotations = nullptr, .annotation_count = 0, .field_type = type_info_of(bar), .struct_offset = __builtin_offsetof(union baz, bar), .is_pointer = false, .is_const = false },
	},
};
