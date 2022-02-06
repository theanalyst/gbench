#include <random>
#include "benchmark/benchmark.h"
#include <algorithm>

namespace {
    std::random_device rd;
    std::mt19937 generator(rd());
}

template <typename T>
auto generate_vec(size_t sz, T start=0,
                  T end = std::numeric_limits<T>::max() - 1)
{
    std::vector<T> v;
    v.resize(sz);
    std::uniform_int_distribution<T> distrib(start, end);
    std::generate(v.begin(), v.end(), [&distrib]() {
        return distrib(generator);
    });
    return v;
}

template <typename C>
size_t count(const C& cont,
             typename C::value_type threshold)
{
    size_t n = 0;
    for (const auto& it: cont) {
        n += bool( it > threshold );
    }
    return n;
}

static void BM_uint32_low_threshold(benchmark::State& state)
{
    auto sz = state.range(0);
    auto vec = generate_vec<uint32_t>(state.range(0));
    for (auto _ : state) {
            benchmark::DoNotOptimize(count(vec, 0));
    }
}

static void BM_uint32_mid_threshold(benchmark::State& state)
{
    auto sz = state.range(0);
    auto vec = generate_vec<uint32_t>(state.range(0));
    for (auto _ : state) {
        benchmark::DoNotOptimize(count(vec, 1<<31));
    }
}

static void BM_uint32_high_threshold(benchmark::State& state)
{
    auto sz = state.range(0);
    auto vec = generate_vec<uint32_t>(state.range(0));
    for (auto _ : state) {
        benchmark::DoNotOptimize(count(vec, std::numeric_limits<uint32_t>::max() - 1));
    }
}


static void BM_uint64_low_threshold(benchmark::State& state)
{
    auto sz = state.range(0);
    auto vec = generate_vec<uint64_t>(state.range(0));
    for (auto _ : state) {
        benchmark::DoNotOptimize(count(vec, 0));
    }
}

static void BM_uint64_mid_threshold(benchmark::State& state)
{
    auto sz = state.range(0);
    auto vec = generate_vec<uint64_t>(state.range(0));
    for (auto _ : state) {
        benchmark::DoNotOptimize(count(vec, 1ULL<<63));
    }
}

static void BM_uint64_high_threshold(benchmark::State& state)
{
    auto sz = state.range(0);
    auto vec = generate_vec<uint64_t>(state.range(0));
    for (auto _ : state) {
        benchmark::DoNotOptimize(count(vec, std::numeric_limits<uint64_t>::max() - 1));
    }
}
int64_t start = 8;
int64_t end = 1UL<<24;
BENCHMARK(BM_uint32_low_threshold)->Range(start, end);
BENCHMARK(BM_uint32_mid_threshold)->Range(start, end);
BENCHMARK(BM_uint32_high_threshold)->Range(start, end);
BENCHMARK(BM_uint64_low_threshold)->Range(start, end);
BENCHMARK(BM_uint64_mid_threshold)->Range(start, end);
BENCHMARK(BM_uint64_high_threshold)->Range(start, end);
BENCHMARK_MAIN();