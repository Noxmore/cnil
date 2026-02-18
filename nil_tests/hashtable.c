#include "hashtable.h"

#include "test.h"
#include "nil/hashtable.h"

static void hashsets() {
	hashset(u32) ints = {};

	hashset_insert(&ints, staticalloc, 5);

	hashtable_foreach(num, &ints) {
		assert_uint32_eq(*num, 5);
	}

	u32 test = 5;
	assert_true(hashset_contains(&ints, &test));
	test = 6;
	assert_false(hashset_contains(&ints, &test));

	test = 5;
	assert_true(hashset_contains(&ints, &test));
	u32 dst;
	hashset_remove_into(&ints, &test, &dst);
	assert_uint32_eq(dst, 5);
	assert_false(hashset_contains(&ints, &test));

	assert_uint32_eq(ints.len, 0);

	hashset_insert(&ints, staticalloc, 5);
	assert_uint32_eq(ints.len, 1);
	hashset_insert(&ints, staticalloc, 6);
	assert_uint32_eq(ints.len, 2);

	test = 5;
	assert_uint32_eq(*hashset_get(&ints, &test), 5);
	test = 6;
	assert_uint32_eq(*hashset_get(&ints, &test), 6);

	hashset_clear(&ints);
	assert_uint32_eq(ints.len, 0);

	hashset_free(&ints, staticalloc);
}

static void hashmaps() {
	hashmap(u32, float) map = {};

	hashmap_insert(&map, staticalloc, 5, 2.5);

	hashtable_foreach(entry, &map) {
		assert_uint32_eq(entry->key, 5);
		assert_float_eq(entry->value, 2.5);
	}

	u32 test = 5;
	assert_true(hashmap_contains(&map, &test));
	test = 6;
	assert_false(hashmap_contains(&map, &test));

	test = 5;
	assert_true(hashmap_contains(&map, &test));
	hashtable_entry_t(&map) dst;
	hashmap_remove_into(&map, &test, &dst);
	assert_uint32_eq(dst.key, 5);
	assert_float_eq(dst.value, 2.5);
	assert_false(hashmap_contains(&map, &test));

	assert_uint32_eq(map.len, 0);

	hashmap_insert(&map, staticalloc, 5, 6.4);
	assert_uint32_eq(map.len, 1);
	hashmap_insert(&map, staticalloc, 6, 9.9);
	assert_uint32_eq(map.len, 2);

	test = 5;
	assert_float_eq(*hashmap_get(&map, &test), 6.4);
	test = 6;
	assert_float_eq(*hashmap_get(&map, &test), 9.9);

	hashmap_clear(&map);
	assert_uint32_eq(map.len, 0);

	hashmap_free(&map, staticalloc);
}

void add_hashtable_tests() {
	START_FILE_TESTS;
	TEST(hashsets);
	TEST(hashmaps);
}
