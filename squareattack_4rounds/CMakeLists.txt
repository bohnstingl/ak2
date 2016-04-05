cmake_minimum_required(VERSION 3.3)
project(squareattack_4rounds)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

set(SOURCE_FILES main.cpp)
add_executable(squareattack_4rounds ${SOURCE_FILES})
target_link_libraries(squareattack_4rounds pthread)