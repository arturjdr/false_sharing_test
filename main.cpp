#include <benchmark/benchmark.h>
#include <cstdlib>
#include <thread>
#include <new>
#include <stdexcept>
#include <iostream>

static void count_odd(unsigned char* start, size_t count, uint32_t *result)
{
	for (size_t i = 0; i < count; i++)
		if ((start[i] % 2) != 0)
			(*result)++;
}

struct unpadded
{
	uint32_t val;
};

#if defined __GNUC__ && __GNUC__ < 12
constexpr auto cacheline = 64;
#else
constexpr auto cacheline = std::hardware_destructive_interference_size;
#endif

static_assert(cacheline > sizeof(uint32_t));
struct padded
{
	uint32_t val;
	// Pad by cacheline size
	uint8_t pad[cacheline - sizeof(uint32_t)];
};

template<typename T>
static void BM_FalseShare(benchmark::State& state) {
	int cores = state.range(0);
	auto res_cont = std::make_unique<uint8_t[]>(cacheline + cores * sizeof(T));
	auto aligned = (uintptr_t)res_cont.get() & (~static_cast<uintptr_t>(cacheline));
	T *results = reinterpret_cast<T *>(aligned);
	//uint32_t *results = std::aligned_alloc(64, cores * sizeof(uint32_t));
	
	int size = 1024*1024*8;
	int chunk = size / cores;
	int last_chunk = size - (chunk * (cores - 1));
	auto data = std::make_unique<uint8_t[]>(size);
	for (size_t i = 0; i < size; i++)
		data[i] = i % 256;
    for (auto _ : state)
	{
		std::vector<std::jthread> tvec;
		tvec.reserve(cores);
		{
			for (size_t i = 0; i < cores - 1; i++)
			{
				results[i].val = 0;
				tvec.push_back(std::jthread(count_odd, data.get() + (chunk * i), chunk, &(results[i].val)));
			}
			results[cores - 1].val = 0;
			tvec.push_back(std::jthread(count_odd, data.get() + (chunk * (cores - 1)), last_chunk, &(results[cores - 1].val)));
			for (size_t i = 0; i < cores; i++)
			{
				benchmark::DoNotOptimize(results[i].val);
			}
		}
		size_t total = 0;
		for (size_t i = 0; i < cores; i++)
		{
			//tvec[i].join();
			tvec[i].join();
			total += results[i].val;
		}
#ifdef CHECK_RESULT
		if (total != size / 2)
		{
			std::cout << "Expected: " << size / 2 << " Total: " << total << std::endl;
			for (size_t i = 0; i < cores; i++)
			{
				std::cout << "Res/" << i << " : " << results[i].val << std::endl;
			}
			throw std::runtime_error("Sum not as expected!");
		}
#endif
	}
}

// Register the function as a benchmark
BENCHMARK(BM_FalseShare<padded>)->Arg(1)->Arg(2)->Arg(4)->Arg(6)->Arg(8);
BENCHMARK(BM_FalseShare<unpadded>)->Arg(1)->Arg(2)->Arg(4)->Arg(6)->Arg(8);


BENCHMARK_MAIN();
