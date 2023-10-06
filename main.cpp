#include <benchmark/benchmark.h>
#include <cstdlib>
#include <thread>

static void count_odd(unsigned char* start, size_t count, uint32_t *result)
{
	for (size_t i = 0; i < count; i++)
		if (start + i != 0)
			(*result)++;
}

static void BM_FalseShare(benchmark::State& state) {
	int cores = 16;
	auto res_cont = std::make_unique<uint8_t[]>(64 + cores * sizeof(uint32_t));
	auto aligned = (uintptr_t)res_cont.get() & (~64ULL);
	uint32_t *results = reinterpret_cast<uint32_t *>(aligned); 
	//uint32_t *results = std::aligned_alloc(64, cores * sizeof(uint32_t));
	std::vector<std::thread> threads;
	threads.reserve(cores);
	int size = 1024*1024*1;
	int chunk = size / cores;
	int last_chunk = size - (chunk * (cores - 1));
	auto data = std::make_unique<uint8_t[]>(size);
	for (size_t i = 0; i < size; i++)
		data[i] = i % 256;
    for (auto _ : state)
	{
		threads.clear();
		for (size_t i = 0; i < cores - 1; i++)
			threads.push_back(std::thread(count_odd, data.get() + (chunk * i), chunk, &(results[i])));
		threads.push_back(std::thread(count_odd, data.get() + (chunk * (cores - 1)), last_chunk, &(results[cores - 1])));
		for (size_t i = 0; i < cores; i++)
		{
			threads[i].join();
			benchmark::DoNotOptimize(results[i]);
		}
	}
}

struct padded
{
	uint32_t val;
	uint8_t pad[60];
};

static void BM_NoShare(benchmark::State& state) {
	int cores = 16;
	auto res_cont = std::make_unique<uint8_t[]>(64 + cores * sizeof(padded));
	auto aligned = (uintptr_t)res_cont.get() & (~64ULL);
	padded *results = reinterpret_cast<padded *>(aligned); 
	//uint32_t *results = std::aligned_alloc(64, cores * sizeof(uint32_t));
	std::vector<std::thread> threads;
	threads.reserve(cores);
	int size = 1024*1024*1;
	int chunk = size / cores;
	int last_chunk = size - (chunk * (cores - 1));
	auto data = std::make_unique<uint8_t[]>(size);
	for (size_t i = 0; i < size; i++)
		data[i] = i % 256;
    for (auto _ : state)
	{
		threads.clear();
		for (size_t i = 0; i < cores - 1; i++)
			threads.push_back(std::thread(count_odd, data.get() + (chunk * i), chunk, &(results[i].val)));
		threads.push_back(std::thread(count_odd, data.get() + (chunk * (cores - 1)), last_chunk, &(results[cores - 1].val)));
		for (size_t i = 0; i < cores; i++)
		{
			threads[i].join();
			benchmark::DoNotOptimize(results[i].val);
		}
	}
}

// Register the function as a benchmark
BENCHMARK(BM_FalseShare);

BENCHMARK(BM_NoShare);

BENCHMARK_MAIN();