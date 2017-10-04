cmake_minimum_required(VERSION 3.6.2)
project(confusepp C CXX)
set(EXECUTABLE_OUTPUT_PATH build)
set(LIBRARY_OUTPUT_PATH build)
set(EXECUTABLE_BUILD_TYPE Debug)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_FLAGS "${CMAKE_C_FLAGS} ${CMAKE_CXX_FLAGS} -Wall -Wextra -Wpedantic")
set(CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

include_directories(include)
file(GLOB SOURCES "src/*cpp" "include/*h")

find_package(Confuse)

include_directories(${CONFUSE_INCLUDE_DIR})

add_library(confusepp ${SOURCES})
add_executable(example_confusepp examples/example_confusepp.cpp)

target_link_libraries(confusepp ${CONFUSE_LIBRARIES})
target_link_libraries(example_confusepp confusepp)
