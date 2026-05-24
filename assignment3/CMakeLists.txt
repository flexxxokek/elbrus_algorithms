cmake_minimum_required(VERSION 3.24)

project(Graph)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED on)

set(CMAKE_BUILD_TYPE Release)

set(SCRIPTS_DIR ${CMAKE_SOURCE_DIR}/scripts)

add_library(mygraph INTERFACE)
target_include_directories(mygraph INTERFACE include)


find_package(GTest REQUIRED)
find_package(benchmark REQUIRED)

# include(FetchContent)
# FetchContent_Declare(
#     googletest
#     GIT_REPOSITORY https://github.com/google/googletest.git
#     GIT_TAG main
# )
# FetchContent_MakeAvailable(googletest)

# set(BENCHMARK_ENABLE_GTEST_TESTS OFF CACHE BOOL "" FORCE)
# set(BENCHMARK_ENABLE_TESTING OFF CACHE BOOL "" FORCE)

# FetchContent_Declare(
#     benchmark
#     GIT_REPOSITORY https://github.com/google/benchmark.git
#     GIT_TAG main
# )
# FetchContent_MakeAvailable(benchmark)

add_executable(run_stdin src/main.cpp)
target_link_libraries(run_stdin 
    PRIVATE mygraph)

add_executable(test_graph test/test.cpp)
target_link_libraries(test_graph
    PRIVATE mygraph
    PRIVATE GTest::gtest_main)

message(AUTHOR_WARNING "Если какие-то ошибки при сборке рекомендуется заменить find_package() на FetchContent() внутри CmakeLists.txt")