cmake_minimum_required(VERSION 3.6)
project(testing)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

set(SOURCE_FILES uthreads.cpp Thread.cpp ex2sanity/test.cpp)
add_executable(testing ${SOURCE_FILES})

set(SOURCE_FILES uthreads.cpp Thread.cpp fb_tests/test1.cpp)
add_executable(test1 ${SOURCE_FILES})

set(SOURCE_FILES uthreads.cpp Thread.cpp fb_tests/test2.cpp)
add_executable(test2 ${SOURCE_FILES})

set(SOURCE_FILES uthreads.cpp Thread.cpp fb_tests/test14.cpp)
add_executable(test14 ${SOURCE_FILES})