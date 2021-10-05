cmake_minimum_required(VERSION 3.10)

set(CMAKE_CXX_STANDARD 17)
option(BENCHMARK_ENABLE_TESTIONG OFF)
project(microbench)
add_subdirectory(benchmark)
add_subdirectory(src)
