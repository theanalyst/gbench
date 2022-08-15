add_executable(findvscount findvscount.cpp)
target_link_libraries(findvscount PRIVATE benchmark::benchmark)

add_executable(strsplit strsplit.cpp)
target_link_libraries(strsplit PRIVATE benchmark::benchmark)

add_executable(randgen randgen.cpp)
target_link_libraries(randgen PRIVATE benchmark::benchmark)

add_executable(strtoint strtoint.cpp)
target_link_libraries(strtoint PRIVATE benchmark::benchmark)

add_executable(mapfilter mapfilter.cpp)
target_link_libraries(mapfilter PRIVATE benchmark::benchmark)

add_executable(xrdstring XrdCppString.cpp XrdOucString.cc)
target_link_libraries(xrdstring PRIVATE benchmark::benchmark)
