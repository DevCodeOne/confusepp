cmake_minimum_required(VERSION 3.2.2)
project(confusepp C CXX)
set(EXECUTABLE_BUILD_TYPE Debug)
set(CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")
add_definitions(-std=c++17)
add_compile_options(-Wall -Wextra -Wpedantic)

option(CONFUSEPP_BUILD_EXAMPLES "Build tests for confusepp" ON)
option(CONFUSEPP_BUILD_TESTS "Build examples for confusepp" ON)

file(GLOB SOURCES "src/*cpp")
add_library(confusepp ${SOURCES})

target_include_directories(confusepp PUBLIC include)
target_include_directories(confusepp PRIVATE ${CONFUSE_INCLUDE_DIR})

find_package(Confuse REQUIRED)

target_link_libraries(confusepp PRIVATE ${CONFUSE_LIBRARIES})
# For std::experimental::filesystem otherwise there are linker errors
target_link_libraries(confusepp PUBLIC stdc++fs)

IF (CONFUSEPP_BUILD_EXAMPLES)
    add_executable(example_confusepp examples/example_confusepp.cpp)
    target_include_directories(example_confusepp PRIVATE include)
    target_link_libraries(example_confusepp confusepp)
    configure_file(examples/example.conf examples/example.conf)
ENDIF()

IF (CONFUSEPP_BUILD_TESTS)
    enable_testing()
    configure_file(tests/tests.conf tests/tests.conf)
    file(GLOB TEST_FILES "tests/catch.hpp" "tests/*cpp")
    add_executable(confusepp_tests ${TEST_FILES})
    target_link_libraries(confusepp_tests confusepp)
    add_test(CatchTests confusepp_tests)
ENDIF()
