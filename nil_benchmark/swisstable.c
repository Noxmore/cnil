#include <stdio.h>

#include "benchmarks.h"

#include "bench.h"

#include "nil/hashtable.h"
#include "cwisstable.h"

CWISS_DECLARE_FLAT_HASHSET(cwiss_hashset, u32);

static void inserting_one_nil() {
	hashset(u32) set = {};
	hashset_insert(&set, staticalloc, 5);
	hashset_free(&set, staticalloc);
}

static void inserting_one_cwiss() {
	cwiss_hashset set = cwiss_hashset_new(NIL_HASHTABLE_GROUP_SIZE);
	constexpr u32 value = 5;
	cwiss_hashset_insert(&set, &value);
	cwiss_hashset_destroy(&set);
}

static void inserting_overflow_nil() {
	hashset(u32) set = {};

	for (u32 i = 0; i < 40; i++)
		hashset_insert(&set, staticalloc, i);

	hashset_free(&set, staticalloc);
}

static void inserting_overflow_cwiss() {
	cwiss_hashset set = cwiss_hashset_new(NIL_HASHTABLE_GROUP_SIZE);

	for (u32 i = 0; i < 40; i++)
		cwiss_hashset_insert(&set, &i);

	cwiss_hashset_destroy(&set);
}

// TODO: more benchmarks


void benchmark_swisstables() {
	BENCH(inserting_one_nil, 1000);
	BENCH(inserting_one_cwiss, 1000);
	BENCH(inserting_overflow_nil, 1000);
	BENCH(inserting_overflow_cwiss, 1000);
}