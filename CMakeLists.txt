cmake_minimum_required(VERSION 3.4)

project(mbs C)

add_subdirectory(argtable3)

file(GLOB_RECURSE SRCS src/*.c)

add_executable(mbs ${SRCS})
