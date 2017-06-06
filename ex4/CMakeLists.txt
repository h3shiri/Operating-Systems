cmake_minimum_required(VERSION 3.6)
project(ex4)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

set(SOURCE_FILES main.cpp CacheFS.h Block.cpp Block.h CachFS.cpp BlockStack.cpp BlockStack.h)
add_executable(ex4 ${SOURCE_FILES})