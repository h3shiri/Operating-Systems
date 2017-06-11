cmake_minimum_required(VERSION 3.6)
project(ex4)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

set(SOURCE_FILES CacheFS.h Block.cpp Block.h CacheFS.cpp BlockStack.cpp BlockStack.h TEST.cpp)
add_executable(TEST ${SOURCE_FILES})

set(SOURCE_FILES CacheFS.h Block.cpp Block.h CacheFS.cpp BlockStack.cpp BlockStack.h Test1.cpp)
add_executable(test1 ${SOURCE_FILES})

set(SOURCE_FILES CacheFS.h Block.cpp Block.h CacheFS.cpp BlockStack.cpp BlockStack.h Test2.cpp)
add_executable(test2 ${SOURCE_FILES})

set(SOURCE_FILES CacheFS.h Block.cpp Block.h CacheFS.cpp BlockStack.cpp BlockStack.h Test3.cpp)
add_executable(test3 ${SOURCE_FILES})

set(SOURCE_FILES CacheFS.h Block.cpp Block.h CacheFS.cpp BlockStack.cpp BlockStack.h Test4.cpp)
add_executable(test4 ${SOURCE_FILES})

set(SOURCE_FILES CacheFS.h Block.cpp Block.h CacheFS.cpp BlockStack.cpp BlockStack.h Test5.cpp)
add_executable(test5 ${SOURCE_FILES})