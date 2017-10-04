cmake_minimum_required(VERSION 3.6)
project(CPP)

set(CMAKE_CXX_STANDARD 11)

set(SOURCE_FILES
    badness.cc)

add_executable(CPP ${SOURCE_FILES})