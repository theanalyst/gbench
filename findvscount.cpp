#include <map>
#include <memory>
#include <tuple>
#include "benchmark/benchmark.h"

static void BM_Count(benchmark::State& state) {
  std::map<std::string, std::string> m;
  auto sz = state.range(0);
  for(auto i =0; i < sz; i++) {
    std::string key = "key" + std::to_string(i);
    std::string val = "val" + std::to_string(i);
    m.emplace(std::make_pair(std::move(key),std::move(val)));
  }

  for (auto _ : state) {
    for (auto i=0; i < sz; i++) {
      std::string key = "key" + std::to_string(std::rand() % sz);
      if(m.count(key)) {
        std::string val = m[key];
      }
    }
  }
}

static void BM_find(benchmark::State& state) {
  std::map<std::string, std::string> m;
  auto sz = state.range(0);
  for(auto i =0; i < sz; i++) {
    std::string key = "key" + std::to_string(i);
    std::string val = "val" + std::to_string(i);
    m.emplace(std::move(key),std::move(val));
  }

  for (auto _ : state) {
    for (auto i=0; i < sz; i++) {
      std::string key = "key" + std::to_string(std::rand() % sz);
      if(auto kv = m.find(key); kv != m.end()) {
        std::string val = kv->second;
      }
    }
  }
}

uint64_t start = 1;
uint64_t end = 1<<24UL;
BENCHMARK(BM_Count)->Range(start, end)->Unit(benchmark::kMillisecond);
BENCHMARK(BM_find)->Range(start, end)->Unit(benchmark::kMillisecond);
BENCHMARK_MAIN();
