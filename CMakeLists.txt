cmake_minimum_required(VERSION 3.19)
project(ch10)

INCLUDE_DIRECTORIES(include/)
AUX_SOURCE_DIRECTORY(src/ SRC_LIST)

set(CMAKE_CXX_STANDARD 17)

add_executable(ch10 main.cpp ${SRC_LIST})