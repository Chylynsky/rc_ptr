#include "benchmark/benchmark.h"

#include <memory>

#include "rc_ptr.hpp"

static void shared_ptr_copy(benchmark::State& state)
{
    std::shared_ptr<int> ptr{ new int{ 0 } };
    for (auto _ : state)
    {
        auto copy = ptr;
        benchmark::DoNotOptimize(copy);
    }
}
BENCHMARK(shared_ptr_copy);

static void rc_ptr_copy(benchmark::State& state)
{
    memory::rc_ptr<int> ptr{ new int{ 0 } };
    for (auto _ : state)
    {
        auto copy = ptr;
        benchmark::DoNotOptimize(copy);
    }
}
BENCHMARK(rc_ptr_copy);

static void raw_ptr_copy(benchmark::State& state)
{
    int x = 0;
    for (auto _ : state)
    {
        auto copy = &x;
        benchmark::DoNotOptimize(copy);
    }
}
BENCHMARK(raw_ptr_copy);
