cmake_minimum_required(VERSION 3.23)
project(MyBrowser)

find_package(GTest REQUIRED)

set(MYSTRUCT_PATH "../../mystruct")

add_library(browser)
target_sources(browser
    PRIVATE
        src/browser.cpp
        
    #логично сделать приватный скоуп, у вас ничего не получится :)
    PUBLIC
    FILE_SET mystruct_header
    TYPE HEADERS
    BASE_DIRS
        ${MYSTRUCT_PATH}
    FILES
        ${MYSTRUCT_PATH}/mystruct.hpp

    PUBLIC
    FILE_SET browser_header
    TYPE HEADERS
    BASE_DIRS
        include
    FILES
        include/browser.hpp
)

add_executable(stdin src/stdin.cpp)
target_link_libraries(stdin browser)


find_package(GTest REQUIRED)

add_executable(run_tests)
target_sources(run_tests
    PRIVATE test/test.cpp)

target_link_libraries(run_tests
    PRIVATE 
    browser
    GTest::gtest_main)


# enable_testing()