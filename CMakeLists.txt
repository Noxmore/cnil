cmake_minimum_required(VERSION 3.31)
project(noost C)

set(CMAKE_C_STANDARD 23)

file(GLOB SRC_FILES ./nil/*.c)

add_library(nil STATIC ${SRC_FILES})

add_executable(tests test.c)