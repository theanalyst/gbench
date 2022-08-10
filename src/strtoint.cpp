#include "benchmark/benchmark.h"
#include <charconv>
#include <system_error>


template <typename StrT, typename NumT>
static auto GetNumeric(const StrT& key, NumT& value,
                       NumT default_val={},
                       std::string* log_msg = nullptr)
  -> std::enable_if_t<std::is_integral_v<NumT>, bool>
{
  NumT result;
  auto ret = std::from_chars(key.data(), key.data() + key.size(), result);

  if (ret.ec != std::errc()) {
    value = default_val;
    if (log_msg != nullptr) {
      auto _ec  = std::make_error_condition(ret.ec);
      // Obligatory gripe about the std; since we can not concat str_view + str
      // doing it this way so that it will work for any str like types
      log_msg->append("\"msg=Failed Numeric conversion\" key=");
      log_msg->append(key);
      log_msg->append(" error_msg=");
      log_msg->append(_ec.message());
    }
    return false;
  }

  value = result;
  return true;
}

static void BM_GetNumeric(benchmark::State& state) {
  uint64_t val;
  std::string s = std::to_string(state.range(0));
  for (auto _: state) {
    benchmark::DoNotOptimize(GetNumeric(s, val));
  }
}

static void BM_atoi(benchmark::State& state) {
  uint64_t val;
  std::string s = std::to_string(state.range(0));
  for (auto _: state) {
    benchmark::DoNotOptimize(atoi(s.c_str()));
  }
}

int64_t start = 8;
int64_t end = 1UL<<24;
BENCHMARK(BM_GetNumeric)->Range(start,end);
BENCHMARK(BM_atoi)->Range(start,end);
BENCHMARK_MAIN();
