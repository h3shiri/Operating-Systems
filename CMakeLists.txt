cmake_minimum_required(VERSION 3.3)
project(Ex1)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -DNDEBUG -Wvla -std=c99")
add_executable(Ex1 ${SOURCE_FILES} osm.h osm.cpp)