cmake_minimum_required(VERSION 3.4)

project(mbs_tests C)

add_executable(mbs_tests main.c ../mbs.c ../argtable3/argtable3.c)
