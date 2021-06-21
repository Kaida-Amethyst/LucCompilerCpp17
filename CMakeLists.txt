cmake_minimum_required(VERSION 3.19)
project(LuaCompiler)

set(CMAKE_CXX_STANDARD 17)

INCLUDE_DIRECTORIES(include/)
AUX_SOURCE_DIRECTORY(src/ SRC_LIST)

add_executable(LuaCompiler main.cpp  ${SRC_LIST})