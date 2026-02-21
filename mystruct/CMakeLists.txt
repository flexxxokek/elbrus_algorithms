cmake_minimum_required(VERSION 3.23)
project(MyStruct)

add_executable(tests)
target_sources(tests
    PRIVATE
    tests.cpp)

target_include_directories(tests PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})

find_package(GTest REQUIRED)
target_link_libraries(tests PRIVATE GTest::gtest_main)
