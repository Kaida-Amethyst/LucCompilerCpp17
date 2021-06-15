cmake_minimum_required(VERSION 3.19)
project(LuaCompiler_renew)

INCLUDE_DIRECTORIES(include/)
AUX_SOURCE_DIRECTORY(src/ SRC_LIST)

set(CMAKE_CXX_STANDARD 17)

add_executable(LuaCompiler_renew main.cpp ${SRC_LIST} luaClosure.cpp luaClosure.h)