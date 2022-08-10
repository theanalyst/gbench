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

BENCHMARK(BM_GetConfigKeys);
BENCHMARK_MAIN();
