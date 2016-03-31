cmake_minimum_required(VERSION 3.3)
project(squareattack_5rounds)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -msse -msse2 -msse3 -O3")

set(SOURCE_FILES main.cpp)
add_executable(squareattack_5rounds ${SOURCE_FILES})
target_link_libraries(squareattack_5rounds pthread)