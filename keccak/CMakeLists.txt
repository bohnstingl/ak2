cmake_minimum_required(VERSION 3.2.2)
project(keccak)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -msse -msse2 -msse3 -O3")

set(SOURCE_FILES find_cubes.cpp keccak.cpp cube.cpp)
add_executable(keccak_find_cubes ${SOURCE_FILES})
target_link_libraries(keccak_find_cubes pthread)