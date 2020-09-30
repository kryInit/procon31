cmake_minimum_required(VERSION 3.16)
project(procon31)

set(CMAKE_C_COMPILER "gcc")
set(CMAKE_CXX_COMPILER "g++")
set(CMAKE_CXX_STANDARD 17)

set(CMAKE_CXX_FLAGS "-O2 -std=c++17 -Wall")

add_subdirectory(solver)
add_subdirectory(procon31App)
