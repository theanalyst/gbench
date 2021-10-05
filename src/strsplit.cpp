#include <string>
#include <deque>
#include <vector>
#include <sstream>
#include "benchmark/benchmark.h"

namespace eos
{
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
}


std::deque<std::string>
insertChunksIntoDeque2(const std::string& path, char delim='/')
{
  size_t start_pos = 0;
  size_t end_pos = path.size();
  std::deque<std::string> dq;
  while (start_pos < path.size() - 1) {
    end_pos = path.find(delim, start_pos);
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

static void BM_split(benchmark::State& state) {
  auto sz = state.range(0);
  std::string s = "/eos/";
  for (auto i = 0; i< sz;i++) {
    s += "folder" + std::to_string(i) + "/";
  }

  for (auto _: state) {

    std::deque<std::string> dq;
    eos::PathProcessor::insertChunksIntoDeque(dq, s);
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


BENCHMARK(BM_split)->DenseRange(1,32,1);
BENCHMARK(BM_split2)->DenseRange(1,32,1);

BENCHMARK_MAIN();
