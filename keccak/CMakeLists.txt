cmake_minimum_required(VERSION 2.8.9)
project(keccak)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -msse -msse2 -msse3 -O3")

set(SOURCE_FILES_FINDCUBES find_cubes.cpp keccak.cpp cube.cpp)
set(SOURCE_FILES_FINDCUBES_EX find_cubes_extended.cpp keccak.cpp cube_extended.cpp)
set(SOURCE_FILES_RECOVER recoverKey.cpp keccak.cpp cube.cpp)

add_executable(recoverKey ${SOURCE_FILES_RECOVER})
add_executable(keccak_find_cubes ${SOURCE_FILES_FINDCUBES})
add_executable(keccak_find_cubes_extended ${SOURCE_FILES_FINDCUBES_EX})
target_link_libraries(keccak_find_cubes pthread)
target_link_libraries(keccak_find_cubes_extended pthread)
target_link_libraries(recoverKey pthread)
