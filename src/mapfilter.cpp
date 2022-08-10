#include "benchmark/benchmark.h"

namespace eos::common {
  //----------------------------------------------------------------------------
//! Transfer a container onto another, this variants destructively move values
//! from other container onto source container at a given pos.
//! \tparam C container type -  will be inferred
//! \param c container where other container will be spliced onto
//! \param other container whose elements will be consumed
//! \param pos position where we need to splice
//----------------------------------------------------------------------------
template <typename C>
void
splice(C& c, C&& other,
       typename C::const_iterator pos)
{
  c.insert(pos,
           std::make_move_iterator(other.begin()),
           std::make_move_iterator(other.end()));
}

//----------------------------------------------------------------------------
//! Transfer a container onto another at the end, this variants destructively move values
//! from other container onto source container at a given pos.
//! \tparam C container type -  will be inferred
//! \param c container where other container will be spliced onto
//! \param other container whose elements will be consumed
//----------------------------------------------------------------------------
template <typename C>
void splice(C& c, C&& other)
{
  splice(c, std::move(other), c.end());
}

//----------------------------------------------------------------------------
//! erase_if that erases elements in place for elements matching a predicate
//! This is useful in erase remove idiom useful for assoc. containers where
//! std::remove_if will not compile, almost borrowed from erase_if C++ ref page
//!
//! @param C the associative container, elements will be removed in place
//! @param pred the predicate to evaluate, please note the container
//!             value_type aka the pair of values will be the input for the
//!             predicate
//! @return the no of elements removed
//! Usage eg:
//!   eos::common::erase_if(m, [](const auto& p){ return p.first % 2 == 0;})
//----------------------------------------------------------------------------
template <typename C, typename Pred>
typename C::size_type
erase_if(C& c, Pred pred)
{
  auto init_sz = c.size();
  for (auto it = c.begin(), last = c.end(); it != last;) {
    if (pred(*it)) {
      it = c.erase(it);
    } else {
      ++it;
    }
  }
  return init_sz - c.size();
}


} // namespace eos::common

struct Policy {
  static std::vector<std::string> GetConfigKeys(const std::string& user_key,
                                                const std::string& group_key,
                                                const std::string& app_key,
                                                bool is_rw,
                                                bool local=false);

  static std::vector<std::string> GetRWConfigKeys(const std::string& key_name,
                                                    const std::string& user_key,
                                                    const std::string& group_key,
                                                    const std::string& app_key);

  static std::string GetRWValue(const std::map<std::string, std::string>& conf_map,
                                const std::string& key_name,
                                const std::string& user_key,
                                const std::string& group_key,
                                const std::string& app_key);

  static std::string getRWkey(const std::string& key_name,
                              bool is_rw,
                              bool is_local=false);

  static const std::vector<std::string> gBasePolicyKeys;
  static const std::vector<std::string> gBasePolicyRWKeys;

};

const std::vector<std::string> Policy::gBasePolicyKeys = {
  "policy.space",
  "policy.layout",
  "policy.nstripes",
  "policy.checksum",
  "policy.blockchecksum",
  "policy.localredirect"
};

static std::string POLICY_BANDWIDTH="policy.bandwidth";
static std::string POLICY_IOPRIORITY="policy.iopriority";
static std::string POLICY_IOTYPE = "policy.iotype";
static std::string POLICY_SCHEDULE="policy.schedule";
const std::vector<std::string> Policy::gBasePolicyRWKeys = {
  "policy.bandwidth",
  "policy.iopriority",
  "policy.iotype",
  "policy.schedule"
};


std::vector<std::string>
Policy::GetConfigKeys(const std::string& user_key,
                      const std::string& group_key,
                      const std::string& app_key,
                      bool is_rw,
                      bool local)
{
  std::string base_prefix;
  std::vector<std::string> config_keys;
  config_keys.reserve(22);
  if (local) {
    base_prefix = "local.";
  }

  // copy elements from base vector, with an optional prefix
  std::transform(gBasePolicyKeys.cbegin(),
                 gBasePolicyKeys.cend(),
                 std::back_inserter(config_keys),
                 [&base_prefix](const std::string& in) {
                   return base_prefix + in;
                 });


  std::string rw_marker = is_rw ? ":w" : ":r";
  for (const auto& _key: gBasePolicyRWKeys) {
    eos::common::splice(config_keys,
                        GetRWConfigKeys(getRWkey(_key, is_rw, local),
                                        user_key, group_key, app_key));
  }

  return config_keys;
}

std::vector<std::string>
Policy::GetRWConfigKeys(const std::string& key_name,
                        const std::string& user_key,
                        const std::string& group_key,
                        const std::string& app_key)
{

  return {
      key_name + app_key,
      key_name + user_key,
      key_name + group_key,
      key_name
  };
}


std::string
Policy::GetRWValue(const std::map<std::string, std::string>& conf_map,
                   const std::string& key_name,
                   const std::string& user_key,
                   const std::string& group_key,
                   const std::string& app_key)
{
  for (const auto& k : GetRWConfigKeys(key_name, user_key, group_key, app_key)) {
    if (const auto& kv = conf_map.find(k);
        kv != conf_map.end() &&
        !kv->second.empty()) {
      return kv->second;
    }
  }
  return {};
}

std::string
Policy::getRWkey(const std::string& key_name, bool is_rw, bool is_local)
{
  std::string base_prefix = is_local ? "local." : "";
  std::string rw_marker = is_rw ? ":w" : ":r";
  return base_prefix + key_name + rw_marker;
}


static void BM_GetConfigKeys(benchmark::State& state) {
  for (auto _: state) {
    benchmark::DoNotOptimize(Policy::GetConfigKeys(".user:user1",
                                                   ".group:group1",
                                                   ".app:app1000",
                                                   true,
                                                   true));
  }
}

struct UserParams {
  std::string user_key;
  std::string group_key;
  std::string app_key;
};

static void BM_PopGetConfigValues(benchmark::State& state) {
  std::map <std::string, std::string> spacepolicies;
  std::vector<std::string> keys;
  bool schedule;
  std::string iopriority, iotype, bandwidth;
  using namespace std::string_literals;
  for (auto _: state) {
    std::string user_key = ".user:user"s + std::to_string(state.range(0));
    std::string group_key = ".group:group"s + std::to_string(state.range(0));
    std::string app_key = ".app:app"s + std::to_string(state.range(0));
    bool rw=true;
    bool is_local=true;
    benchmark::DoNotOptimize(keys =Policy::GetConfigKeys(user_key,
                                                         group_key,
                                                         app_key,
                                                         true,
                                                         true));
    for (const auto& k: keys) {
      benchmark::DoNotOptimize(spacepolicies.insert_or_assign(k,
                                                              "dummy" + std::to_string(state.range(0))));
    }

    benchmark::DoNotOptimize(schedule = Policy::GetRWValue(spacepolicies, Policy::getRWkey(POLICY_SCHEDULE, rw, is_local),
                                                           user_key, group_key, app_key) == "1");
    benchmark::DoNotOptimize(iopriority = Policy::GetRWValue(spacepolicies, Policy::getRWkey(POLICY_IOPRIORITY, rw, is_local),
                                                             user_key, group_key, app_key));
    benchmark::DoNotOptimize(iotype = Policy::GetRWValue(spacepolicies, Policy::getRWkey(POLICY_IOTYPE, rw, is_local),
                                                         user_key, group_key, app_key));
    benchmark::DoNotOptimize(bandwidth = Policy::GetRWValue(spacepolicies, Policy::getRWkey(POLICY_BANDWIDTH, rw, is_local),
                                                            user_key, group_key, app_key));
  }

}

static void BM_GetConfigValues(benchmark::State& state) {
  std::map <std::string, std::string> spacepolicies;
  std::vector<std::string> keys;
  bool schedule;
  std::string iopriority, iotype, bandwidth;
  using namespace std::string_literals;
  std::string user_key = ".user:user"s + std::to_string(state.range(0));
  std::string group_key = ".group:group"s + std::to_string(state.range(0));
  std::string app_key = ".app:app"s + std::to_string(state.range(0));
  bool rw=true;
  bool is_local=true;
  keys =Policy::GetConfigKeys(user_key,
                              group_key,
                              app_key,
                              true,
                              true);
  for (const auto& k: keys) {
    if ((k.find(user_key) != std::string::npos) ||
        (k.find(group_key) != std::string::npos) ||
        (k.find(app_key) != std::string::npos)) {
      spacepolicies.insert_or_assign(k,
                                     "dummy" + std::to_string(state.range(0)));

    } else {
      spacepolicies.insert_or_assign(k, "");
    }
  }

  for (auto _: state) {
    benchmark::DoNotOptimize(schedule = Policy::GetRWValue(spacepolicies, Policy::getRWkey(POLICY_SCHEDULE, rw, is_local),
                                                           user_key, group_key, app_key) == "1");
    benchmark::DoNotOptimize(iopriority = Policy::GetRWValue(spacepolicies, Policy::getRWkey(POLICY_IOPRIORITY, rw, is_local),
                                                             user_key, group_key, app_key));
    benchmark::DoNotOptimize(iotype = Policy::GetRWValue(spacepolicies, Policy::getRWkey(POLICY_IOTYPE, rw, is_local),
                                                         user_key, group_key, app_key));
    benchmark::DoNotOptimize(bandwidth = Policy::GetRWValue(spacepolicies, Policy::getRWkey(POLICY_BANDWIDTH, rw, is_local),
                                                            user_key, group_key, app_key));
  }

}

static void BM_GetConfigValuesErase(benchmark::State& state) {
  std::map <std::string, std::string> spacepolicies;
  std::vector<std::string> keys;
  bool schedule;
  std::string iopriority, iotype, bandwidth;
  using namespace std::string_literals;
  std::string user_key = ".user:user"s + std::to_string(state.range(0));
  std::string group_key = ".group:group"s + std::to_string(state.range(0));
  std::string app_key = ".app:app"s + std::to_string(state.range(0));
  bool rw=true;
  bool is_local=true;
  keys =Policy::GetConfigKeys(user_key,
                              group_key,
                              app_key,
                              true,
                              true);
  for (const auto& k: keys) {
    if ((k.find(user_key) != std::string::npos) ||
        (k.find(group_key) != std::string::npos) ||
        (k.find(app_key) != std::string::npos)) {
      spacepolicies.insert_or_assign(k,
                                     "dummy" + std::to_string(state.range(0)));

    } else {
      spacepolicies.insert_or_assign(k, "");
    }
  }

  for (auto _: state) {
    eos::common::erase_if(spacepolicies, [](const auto& kv) {
      return kv.second.empty();
    });
    benchmark::DoNotOptimize(schedule = Policy::GetRWValue(spacepolicies, Policy::getRWkey(POLICY_SCHEDULE, rw, is_local),
                                                           user_key, group_key, app_key) == "1");
    benchmark::DoNotOptimize(iopriority = Policy::GetRWValue(spacepolicies, Policy::getRWkey(POLICY_IOPRIORITY, rw, is_local),
                                                             user_key, group_key, app_key));
    benchmark::DoNotOptimize(iotype = Policy::GetRWValue(spacepolicies, Policy::getRWkey(POLICY_IOTYPE, rw, is_local),
                                                         user_key, group_key, app_key));
    benchmark::DoNotOptimize(bandwidth = Policy::GetRWValue(spacepolicies, Policy::getRWkey(POLICY_BANDWIDTH, rw, is_local),
                                                            user_key, group_key, app_key));

  }

}



BENCHMARK(BM_GetConfigKeys);
BENCHMARK(BM_PopGetConfigValues)->Range(1,1<<20);
BENCHMARK(BM_GetConfigValues)->Range(1,1<<20);
BENCHMARK(BM_GetConfigValuesErase)->Range(1,1<<20);

BENCHMARK_MAIN();
