cmake_minimum_required(VERSION 3.9)
project(Receiver)

set(CMAKE_CXX_STANDARD 11)

add_executable(Receiver main.cpp TCP_Header.cpp TCP_Header.h)