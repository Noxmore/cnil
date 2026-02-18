#pragma once

#include "nil/nint.h"

typedef void (*bench_fn)();

void bench(const char* name, bench_fn fn, usize iterations);

#define BENCH(FN, ITERATIONS) bench(#FN, FN, ITERATIONS)