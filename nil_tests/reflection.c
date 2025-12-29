#include "reflection.h"
#include "test.h"

/*static void assert_field_eq(const type_info_field* field, type_info_field other) {

}*/

static void ensure_complex_type() {
	const type_info* type = TYPE_INFO(complex_type);

	assert_int_eq(type->kind, type_info_struct);
	assert_true(type->mutable);
	assert_str_eq(type->name.data, "complex_type");

	assert_uint_eq(type->size, sizeof(complex_type));
	assert_uint_eq(type->align, alignof(complex_type));

	assert_uint_eq(type->annotation_count, 1);
	assert_str_eq(type->annotations[0].data, "type_annotation");

	// assert_field_eq(&type->struct_data.fields[0], (type_info_field)DEFINE_TYPE_INFO_FIELD(complex_type, int, foo));
	// type->struct_data.fields[0]
}

void add_reflection_tests() {
	START_FILE_TESTS;
	TEST(ensure_complex_type);
}
