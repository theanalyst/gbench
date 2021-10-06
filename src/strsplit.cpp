#include <string>
#include <deque>
#include <vector>
#include <sstream>
#include <cstring>
#include <sstream>
#include "split.h"
#include "lazysplit.hpp"
#include "benchmark/benchmark.h"
#include <iostream>
//----------------------------------------------------------------------------
//! Helper class responsible for spliting the path
//----------------------------------------------------------------------------
class PathProcessor
{
public:

  //------------------------------------------------------------------------
  //! Split the path and prepend its elements into a deque.
  //------------------------------------------------------------------------
  static void insertChunksIntoDeque(std::deque<std::string>& elements,
                                    const std::string& path)
  {
    std::vector<std::string> tmp;
    splitPath(tmp, path);

    for(auto it = tmp.rbegin(); it != tmp.rend(); it++) {
      elements.push_front(*it);
    }
  }

  //------------------------------------------------------------------------
  //! Split the path and put its elements in a vector, the tokens are
  //! copied, the buffer is not overwritten
  //------------------------------------------------------------------------
  static void splitPath(std::vector<std::string>& elements,
                        const std::string& path)
  {
    elements.clear();
    std::vector<char*> elems;
    char buffer[path.length() + 1];
    strcpy(buffer, path.c_str());
    splitPath(elems, buffer);

    for (size_t i = 0; i < elems.size(); ++i) {
      elements.push_back(elems[i]);
    }
  }

  //------------------------------------------------------------------------
  //! Split the path and put its element in a vector, the split is done
  //! in-place and the buffer is overwritten
  //------------------------------------------------------------------------
  static void splitPath(std::vector<char*>& elements, char* buffer)
  {
    elements.clear();
    elements.reserve(10);
    char* cursor = buffer;
    char* beg    = buffer;

    //----------------------------------------------------------------------
    // Go by the characters one by one
    //----------------------------------------------------------------------
    while (*cursor) {
      if (*cursor == '/') {
        *cursor = 0;

        if (beg != cursor) {
          elements.push_back(beg);
        }

        beg = cursor + 1;
      }

      ++cursor;
    }

    if (beg != cursor) {
      elements.push_back(beg);
    }
  }

  //------------------------------------------------------------------------
  //! Absolute Path sanitizing all '/../' and '/./' entries
  //------------------------------------------------------------------------
  static void absPath(std::string& mypath)
  {
    std::vector<std::string> elements, abs_path;
    splitPath(elements, mypath);
    std::ostringstream oss;
    int skip = 0;

    for (auto it = elements.rbegin(); it != elements.rend(); ++it) {
      if ((*it == ".") || it->empty()) {
        continue;
      }

      if (*it == "..") {
        ++skip;
        continue;
      }

      if (skip) {
        --skip;
        continue;
      }

      abs_path.push_back(*it);
    }

    for (auto it = abs_path.rbegin(); it != abs_path.rend(); ++it) {
      oss << "/" << *it;
    }

    mypath = oss.str();

    if (mypath.empty()) {
      mypath = "/";
    }
  }
};


std::vector<std::string>
insertChunksIntoDeque2(std::string_view path, std::string_view delim="/")
{
  size_t start_pos = 0;
  size_t end_pos = path.size();
  std::vector<std::string> dq;
  //dq.reserve(end_pos / 2);
  while (start_pos < path.size() - 1) {
    end_pos = path.find_first_of(delim, start_pos);
    if (end_pos == std::string::npos) {
      dq.emplace_back(path.substr(start_pos));
      break;
    }
    if (end_pos == start_pos) {
      start_pos++;
      continue;
    }
    dq.emplace_back(path.substr(start_pos, end_pos - start_pos));
    start_pos = end_pos + 1;
  }
  return dq;
}

inline std::vector<std::string> split(std::string data, std::string token)
{
  std::vector<std::string> output;
  size_t pos = std::string::npos;

  do {
    pos = data.find(token);
    output.push_back(data.substr(0, pos));

    if (std::string::npos != pos) {
      data = data.substr(pos + token.size());
    }
  } while (std::string::npos != pos);

  return output;
}

template<typename C>
C StringTokenizer_split(const std::string& str, char delimiter)
{
  std::istringstream iss(str);
  C container;
  std::string part;

  while (std::getline(iss, part, delimiter)) {
    if (!part.empty()) {
      container.emplace_back(std::move(part));
    }
  }

  return container;
}

static void BM_split(benchmark::State& state) {
  auto sz = state.range(0);
  std::string s = "/eos/";
  for (auto i = 0; i< sz;i++) {
    s += "folder" + std::to_string(i) + "/";
  }

  for (auto _: state) {

    std::deque<std::string> dq;
    PathProcessor::insertChunksIntoDeque(dq, s);
  }
}

static void BM_split2(benchmark::State& state) {
  auto sz = state.range(0);
  std::string s = "/eos/";
  for (auto i = 0; i< sz;i++) {
    s += "folder" + std::to_string(i) + "/";
  }

  for (auto _: state) {
    auto dq2 = insertChunksIntoDeque2(s);
  }
}

static void BM_splitv(benchmark::State& state) {
  auto sz = state.range(0);
  std::string s = "/eos/";
  for (auto i = 0; i< sz;i++) {
    s += "folder" + std::to_string(i) + "/";
  }

  for (auto _: state) {
    auto v = split(s,"/");
  }
}

static void BM_split_t(benchmark::State& state) {
  auto sz = state.range(0);
  std::string s = "/eos/";
  for (auto i = 0; i< sz;i++) {
    s += "folder" + std::to_string(i) + "/";
  }

  for (auto _: state) {
    auto v = StringTokenizer_split<std::vector<std::string>>(s,'/');
  }
}

static void BM_splitc(benchmark::State& state) {
  auto sz = state.range(0);
  std::string s = "/eos/";
  for (auto i = 0; i< sz;i++) {
    s += "folder" + std::to_string(i) + "/";
  }

  for (auto _: state) {
    auto parts = utils::split(s,"/");
    std::vector<std::string> result;
    result.assign(parts.begin(), parts.end());
  }
}

static void BM_splite(benchmark::State& state) {
  auto sz = state.range(0);
  std::string s = "/eos/";
  for (auto i = 0; i< sz;i++) {
    s += "folder" + std::to_string(i) + "/";
  }

  for (auto _: state) {
    auto parts = eos::common::LazySplit<std::string_view,std::string_view>(s, "/");

    std::vector<std::string_view> result;
    for (std::string_view it: parts) {
      result.emplace_back(it);
    }
    //result.assign(parts.begin(), parts.end());
  }
}

static void BM_splitec(benchmark::State& state) {
  auto sz = state.range(0);
  std::string s = "/eos/";
  for (auto i = 0; i< sz;i++) {
    s += "folder" + std::to_string(i) + "/";
  }

  for (auto _: state) {
    auto parts = eos::common::LazySplit<std::string_view,std::string_view>(s, "/");

    std::vector<std::string> result;
    for (std::string_view it: parts) {
      result.emplace_back(it);
    }
    //result.assign(parts.begin(), parts.end());
  }
}


static void BM_CopyDeque(benchmark::State& state) {
  auto sz = state.range(0);
  std::deque<std::string> dq;
  for (int i=0; i< sz; i++) {
    dq.emplace_back("folder"+std::to_string(sz));
  }
  for (auto _ : state) {
    auto dq2 = dq;
  }
}

static void BM_MoveDeque(benchmark::State& state) {
  auto sz = state.range(0);
  std::deque<std::string> dq;
  for (int i=0; i< sz; i++) {
    dq.emplace_back("folder"+std::to_string(sz));
  }
  for (auto _ : state) {
    auto dq2 = std::move(dq);
  }
}


BENCHMARK(BM_split)->DenseRange(0,32,4);
BENCHMARK(BM_split2)->DenseRange(0,32,4);
BENCHMARK(BM_splitv)->DenseRange(0,32,4);
BENCHMARK(BM_split_t)->DenseRange(0,32,4);
BENCHMARK(BM_splitc)->DenseRange(0,32,4);
BENCHMARK(BM_splite)->DenseRange(0,32,4);
BENCHMARK(BM_splitec)->DenseRange(0,32,4);
BENCHMARK_MAIN();
