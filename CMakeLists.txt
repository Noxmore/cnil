# TODO: Include https://github.com/taviso/swisstable?

cmake_minimum_required(VERSION 3.31)
project(cnil C)

set(CMAKE_C_STANDARD 23)
set(CMAKE_EXPORT_COMPILE_COMMANDS)

#include(FetchContent)
#set(FETCHCONTENT_QUIET FALSE)
#FetchContent_Declare(
#	swisstable
#	GIT_REPOSITORY https://github.com/taviso/swisstable.git
#	GIT_TAG d01faf6
#)
#FetchContent_MakeAvailable(swisstable)

file(GLOB SRC_FILES ./nil/*.c)

add_library(nil STATIC ${SRC_FILES})

file(GLOB REFLECTOR_SRC_FILES ./nil_reflector/*.c)

find_package(Clang CONFIG REQUIRED)
add_executable(nil_reflector ${REFLECTOR_SRC_FILES})
target_include_directories(nil_reflector PRIVATE ${CLANG_INCLUDE_DIRS})
target_link_libraries(nil_reflector PRIVATE libclang)
target_link_libraries(nil_reflector PRIVATE nil)
target_include_directories(nil_reflector PRIVATE .)

function(reflect_types target headers)
	set(generated_sources "")
	foreach(header ${headers})
		get_filename_component(filename "${header}" NAME_WE)
		set(out "${CMAKE_CURRENT_BINARY_DIR}/${filename}_reflected.c")

		add_custom_command(
			OUTPUT "${out}"
			COMMAND $<TARGET_FILE:nil_reflector> "${header}" "${out}"
			DEPENDS "${header}"
			COMMENT "Reflecting ${header} -> ${out}"
			VERBATIM
		)

		list(APPEND generated_sources "${out}")
	endforeach()

	add_custom_target(reflect_${target} DEPENDS ${generated_sources})
	add_dependencies(reflect_${target} nil_reflector)
	add_dependencies(${target} reflect_${target})
endfunction()

# For debugging memory issues
#if (CMAKE_BUILD_TYPE STREQUAL "Debug")
#	target_compile_options(nil_reflector PRIVATE -fsanitize=address)
#	target_link_options(nil_reflector PRIVATE -fsanitize=address)
#endif()
#target_link_libraries(nil PUBLIC swisstable)

# TODO: we should remove this or find a better way?
#add_executable(tests test.c)
#target_link_libraries(tests nil)
