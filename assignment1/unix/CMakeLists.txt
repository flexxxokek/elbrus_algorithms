cmake_minimum_required(VERSION 3.23)
project(MyUnix)

set(MYSTRUCT_PATH "../../mystruct")

add_library(unix)
target_sources(unix
    PRIVATE
        src/unix.cpp
        
    #логично сделать приватный скоуп
    PUBLIC
    FILE_SET mystruct_header
    TYPE HEADERS
    BASE_DIRS
        ${MYSTRUCT_PATH}
    FILES
        ${MYSTRUCT_PATH}/mystruct.hpp

    PUBLIC
    FILE_SET unix_header
    TYPE HEADERS
    BASE_DIRS
        include
    FILES
        include/unix.hpp
)

add_executable(stdin src/stdin.cpp)
target_link_libraries(stdin unix)

find_package(GTest REQUIRED)

add_executable(run_tests)
target_sources(run_tests
    PRIVATE test/test.cpp)

target_link_libraries(run_tests
    PRIVATE 
    unix
    GTest::gtest_main)


# enable_testing()