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

file(GLOB SRC_FILES CONFIGURE_DEPENDS nil/*.c)

add_library(nil STATIC ${SRC_FILES})
target_include_directories(nil PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

file(GLOB REFLECTOR_SRC_FILES CONFIGURE_DEPENDS nil_reflector/*.c)

find_package(Clang CONFIG REQUIRED)
add_executable(nil_reflector ${REFLECTOR_SRC_FILES})
target_include_directories(nil_reflector PRIVATE ${CLANG_INCLUDE_DIRS})
target_link_libraries(nil_reflector PRIVATE libclang)
target_link_libraries(nil_reflector PRIVATE nil)
target_include_directories(nil_reflector PRIVATE .)

function(reflect_types target)
	set(generated_sources "")
	foreach(header ${ARGN})
		get_filename_component(filename "${header}" NAME_WE)
		set(out "${CMAKE_CURRENT_BINARY_DIR}/${filename}.reflection.c")

		set(generator_flags
			"$$<LIST:TRANSFORM,$<TARGET_PROPERTY:${target},INCLUDE_DIRECTORIES>,PREPEND,-I>"
			"$$<LIST:TRANSFORM,$<TARGET_PROPERTY:${target},COMPILE_DEFINITIONS>,PREPEND,-D>"
			"$$<LIST:TRANSFORM,$<TARGET_PROPERTY:${target},COMPILE_OPTIONS>,>"
		)

		add_custom_command(
			OUTPUT "${out}"
			COMMAND $<TARGET_FILE:nil_reflector> -std=c23 ${generator_flags} "${header}" "${out}"
			DEPENDS "${header}" nil_reflector
			COMMENT "Reflecting ${header} -> ${out}"
			COMMAND_EXPAND_LISTS
			VERBATIM
		)

		list(APPEND generated_sources "${out}")
	endforeach()

	target_sources(${target} PRIVATE ${generated_sources})
endfunction()

#	add_library(${reflected_target} OBJECT ${generated_sources})
#	add_custom_target(${reflected_target} DEPENDS ${generated_sources})
#	add_dependencies(${reflected_target} nil_reflector)
#	add_dependencies(${target} ${reflected_target})
#	target_link_libraries(${target} PRIVATE ${reflected_target})

# For debugging memory issues
#if (CMAKE_BUILD_TYPE STREQUAL "Debug")
#	target_compile_options(nil_reflector PRIVATE -fsanitize=address)
#	target_link_options(nil_reflector PRIVATE -fsanitize=address)
#endif()
#target_link_libraries(nil PUBLIC swisstable)

# TODO: we should remove this or find a better way?
#add_executable(tests test.c)
#target_link_libraries(tests nil)
