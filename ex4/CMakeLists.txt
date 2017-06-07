cmake_minimum_required(VERSION 3.6)
project(ex4)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

set(SOURCE_FILES CacheFS.h Block.cpp Block.h CacheFS.cpp BlockStack.cpp BlockStack.h TEST.cpp)
add_executable(test1 ${SOURCE_FILES})

