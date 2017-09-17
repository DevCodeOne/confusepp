cmake_minimum_required(VERSION 3.6.2)
project(confusepp C CXX)
set(EXECUTABLE_OUTPUT_PATH build)
set(EXECUTABLE_BUILD_TYPE Debug)
set(CMAKE_CXX_FLAGS "${CMAKE_C_FLAGS} ${CMAKE_CXX_FLAGS} -Wall -Wextra -Wpedantic")
set(CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")
add_definitions(-std=c++1z)

include_directories(include)
file(GLOB SOURCES "src/*cpp" "include/*h")

find_package(Confuse)

include_directories(${CONFUSE_INCLUDE_DIR})

add_executable(confusepp examples/example_main.cpp ${SOURCES})

target_link_libraries(confusepp ${CONFUSE_LIBRARIES})
