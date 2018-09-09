cmake_minimum_required(VERSION 3.4)

project(mbs C)

add_subdirectory(src/argtable3)

file(GLOB SRCS src/*.c)

find_package(Curses REQUIRED)

include_directories(${CURSES_INCLUDE_DIRS})

add_executable(mbs ${SRCS} src/argtable3/argtable3.c)
target_link_libraries(mbs ${CURSES_LIBRARIES})

install(TARGETS mbs DESTINATION bin)
