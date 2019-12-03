cmake_minimum_required(VERSION 3.15)
project(cs352)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -pthread")

add_executable(cs352 main.cpp)