cmake_minimum_required(VERSION 3.6)
project(fresh_ex2)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

set(SOURCE_FILES main.cpp Thread.cpp uthreads.cpp)
add_executable(fresh_ex2 ${SOURCE_FILES})