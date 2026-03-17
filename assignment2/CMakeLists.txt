cmake_minimum_required(VERSION 3.24)

project(MySort)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED on)

set(CMAKE_BUILD_TYPE Release)

set(SCRIPTS_DIR ${CMAKE_SOURCE_DIR}/scripts)

add_library(mysort INTERFACE)
target_include_directories(mysort INTERFACE include)


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

add_executable(test_sort test/test.cpp)
target_link_libraries(test_sort
    PRIVATE mysort
    PRIVATE GTest::gtest_main)


add_executable(bench_sort benchmark/benchmark.cpp)
target_link_libraries(bench_sort
    PRIVATE mysort
    PRIVATE benchmark::benchmark
)

set(BENCHMARK_RESULT_DIR ${CMAKE_BINARY_DIR}/benchmark_result)
set(PLOTS_DIR ${CMAKE_SOURCE_DIR}/plots)

add_custom_target(graph_task1
    COMMAND ${CMAKE_COMMAND} -E make_directory ${BENCHMARK_RESULT_DIR}
    COMMAND
        $<TARGET_FILE:bench_sort>
            --benchmark_filter="BubbleSort|BubbleSortWithCondition|ShakerSort|CombSort"
            --benchmark_out=${BENCHMARK_RESULT_DIR}/results_slow.json
            --benchmark_out_format=json
    COMMAND
        python3 ${SCRIPTS_DIR}/plot_bench.py
            ${BENCHMARK_RESULT_DIR}/results_slow.json
            --out ${PLOTS_DIR}
    COMMENT "Running slow sorts (Bubble, Shaker, Comb) → plots"
    DEPENDS bench_sort
    BYPRODUCTS ${BENCHMARK_RESULT_DIR}/results_slow.json
)

add_custom_target(graph_task2
    COMMAND ${CMAKE_COMMAND} -E make_directory ${BENCHMARK_RESULT_DIR}
    COMMAND
        $<TARGET_FILE:bench_sort>
            --benchmark_filter="RadixSortBase10|RadixSortBase20|RadixSortBase100|StdSort"
            --benchmark_out=${BENCHMARK_RESULT_DIR}/results_fast.json
            --benchmark_out_format=json
    COMMAND
        python3 ${SCRIPTS_DIR}/plot_bench.py
            ${BENCHMARK_RESULT_DIR}/results_fast.json
            --out ${PLOTS_DIR}
    COMMENT "Running fast sorts (Radix, std::sort) → plots"
    DEPENDS bench_sort
    BYPRODUCTS ${BENCHMARK_RESULT_DIR}/results_fast.json
)

add_custom_target(graph
    DEPENDS graph_task1 graph_task2
)


message(AUTHOR_WARNING "Если какие-то ошибки при сборке рекомендуется заменить find_package() на FetchContent() внутри CmakeLists.txt")