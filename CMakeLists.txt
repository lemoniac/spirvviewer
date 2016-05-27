cmake_minimum_required (VERSION 2.6)
project (spirvviewer)

set (CMAKE_CXX_FLAGS "-g --std=gnu++11 ${CMAKE_CXX_FLAGS}")

add_executable (spirvparser
    spirvparser.cpp
    Type.cpp)

add_executable (test
    test_microcore.cpp
    Microcore.cpp)
