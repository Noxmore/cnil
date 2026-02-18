#include "bench.h"

#include <stdio.h>
#include <time.h>

/*static struct formatted { double value; const char* postfix; } format_time(double seconds) {
	if (seconds < 0.001) {
		return (struct formatted){ seconds * 1e+9, "ns" };
	}
	if (seconds < 1) {
		return (struct formatted){ seconds * 1000, "ms" };
	}

	return (struct formatted){ seconds, "s" };
}*/

void bench(const char* name, bench_fn fn, usize iterations) {
	clock_t min_clocks = -1;
	clock_t total_clocks = 0;
	clock_t max_clocks = 0;

	// Warm up first.
	for (u32 i = 0; i < 100; i++)
		fn();

	for (u32 i = 0; i < iterations; i++) {
		const clock_t start = clock();
		fn();
		const clock_t elapsed = clock() - start;

		if (elapsed < min_clocks)
			min_clocks = elapsed;

		if (elapsed > max_clocks)
			max_clocks = elapsed;

		total_clocks += elapsed;
	}

	const double min = ((double)min_clocks / (double)CLOCKS_PER_SEC) * 1000;
	const double average = ((double)total_clocks / (double)CLOCKS_PER_SEC / (double)iterations) * 1000;
	const double max = ((double)max_clocks / (double)CLOCKS_PER_SEC) * 1000;

	printf("[%s] min: %fms, average: %fms, max: %fms\n", name, min, average, max);
}
