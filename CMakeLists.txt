cmake_minimum_required(VERSION 3.2.2)
project(confusepp C CXX)
set(EXECUTABLE_BUILD_TYPE Debug)
add_compile_options(-Wall -Wextra -Wpedantic)
add_definitions(-std=c++1z)
set(CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

include_directories(include)
file(GLOB SOURCES "src/*cpp" "include/*h")
configure_file(examples/example.conf examples/example.conf)

find_package(Confuse REQUIRED)

include_directories(${CONFUSE_INCLUDE_DIR})

add_library(confusepp ${SOURCES})
add_executable(example_confusepp examples/example_confusepp.cpp)

target_link_libraries(confusepp ${CONFUSE_LIBRARIES})
target_link_libraries(example_confusepp confusepp)

enable_testing()
configure_file(tests/tests.conf tests/tests.conf)
file(GLOB TEST_FILES "tests/catch.hpp" "tests/*cpp")
add_executable(confusepp_tests ${TEST_FILES})
target_link_libraries(confusepp_tests confusepp)
add_test(CatchTests confusepp_tests)
