cmake_minimum_required(VERSION 3.31)
project(cnil C)

set(CMAKE_C_STANDARD 23)
set(CMAKE_EXPORT_COMPILE_COMMANDS)

file(GLOB SRC_FILES ./nil/*.c)

add_library(nil STATIC ${SRC_FILES})

add_executable(tests test.c)
target_link_libraries(tests nil)
