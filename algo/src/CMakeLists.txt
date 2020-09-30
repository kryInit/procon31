cmake_minimum_required(VERSION 3.16)
project(solver)


set(CMAKE_C_COMPILER "gcc")
set(CMAKE_CXX_COMPILER "g++")
set(CMAKE_CXX_STANDARD 17)

set(CMAKE_CXX_FLAGS "-O2 -std=c++17 -Wall")

add_executable(procon31.app procon31.cpp)
add_executable(solver solver.cpp)

set_target_properties( procon31.app
    PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "../../../"
)

set_target_properties( solver
    PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "../../bin/"
)
