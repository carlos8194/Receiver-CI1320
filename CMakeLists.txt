cmake_minimum_required(VERSION 3.9)
project(Receiver)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -pthread")

add_executable(Receiver main.cpp TCP_Header.cpp TCP_Header.h)