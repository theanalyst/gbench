add_executable(findvscount findvscount.cpp)
target_link_libraries(findvscount PRIVATE benchmark::benchmark)

add_executable(strsplit strsplit.cpp)
target_link_libraries(strsplit PRIVATE benchmark::benchmark)

add_executable(randgen randgen.cpp)
target_link_libraries(randgen PRIVATE benchmark::benchmark)
