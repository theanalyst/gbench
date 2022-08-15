#include "XrdOucString.hh"
#include <string>
#include "benchmark/benchmark.h"

static void BM_StringCreate(benchmark::State& state)
{
  const char* s = "foobar";
  for (auto _: state) {
    benchmark::DoNotOptimize(std::string(s));
  }
}

static void BM_XrdStringCreate(benchmark::State& state)
{
  const char* s = "foobar";
  for (auto _: state) {
    benchmark::DoNotOptimize(XrdOucString(s));
  }
}


BENCHMARK(BM_StringCreate);
BENCHMARK(BM_XrdStringCreate);
BENCHMARK_MAIN();
