#include "XrdOucString.hh"
#include <string>
#include <cstring>
#include "benchmark/benchmark.h"

#define STR(X) #X
#define REP2(X) X X
#define REP4(X) REP2(X) REP2(X)
#define REP8(X) REP8(X) REP8(X)


static void BM_StringCreate(benchmark::State& state)
{
  std::string _s(state.range(0), 'a');
  const char* s = _s.c_str();
  for (auto _: state) {
    benchmark::DoNotOptimize(std::string(s));
  }
}

static void BM_XrdStringCreate(benchmark::State& state)
{
  std::string _s(state.range(0), 'a');
  const char* s = _s.c_str();

  for (auto _: state) {
    benchmark::DoNotOptimize(XrdOucString(s));
  }
}


static void BM_StringAppend(benchmark::State& state)
{
  std::string s("This is a line");
  for (auto _: state) {
    for (size_t i=0; i<state.range(0); ++i)
      benchmark::DoNotOptimize(s += "a");
  }
}

static void BM_XrdStringAppend(benchmark::State& state)
{
  XrdOucString s("This is a line");
  for (auto _: state) {
    for (size_t i=0; i<state.range(0); ++i)
      benchmark::DoNotOptimize(s += "a");
  }
}


BENCHMARK(BM_StringCreate)->RangeMultiplier(2)->Range(8,1<<10);
BENCHMARK(BM_XrdStringCreate)->RangeMultiplier(2)->Range(8,1<<10);
BENCHMARK(BM_StringAppend)->RangeMultiplier(2)->Range(8,1<<10);
BENCHMARK(BM_XrdStringAppend)->RangeMultiplier(2)->Range(8,1<<10);

BENCHMARK_MAIN();
