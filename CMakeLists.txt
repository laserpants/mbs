cmake_minimum_required(VERSION 3.4)

project(mbs C)

enable_testing()

add_subdirectory(src/argtable3)

file(GLOB SRCS src/*.c)

find_package(Curses REQUIRED)

include_directories(${CURSES_INCLUDE_DIRS})

add_executable(mbs ${SRCS} src/argtable3/argtable3.c)
target_link_libraries(mbs ${CURSES_LIBRARIES})

add_executable(mbs_tests src/tests/main.c src/mbs.c src/argtable3/argtable3.c)

install(TARGETS mbs DESTINATION bin)
add_test(mbs_tests mbs_tests)
