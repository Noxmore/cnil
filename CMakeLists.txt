cmake_minimum_required(VERSION 3.31)
project(cnil C)

set(CMAKE_C_STANDARD 23)
set(CMAKE_EXPORT_COMPILE_COMMANDS)

option(NIL_TESTS, "Nil Tests")

#include(FetchContent)
#set(FETCHCONTENT_QUIET FALSE)
#FetchContent_Declare(
#	mlib
#	GIT_REPOSITORY https://github.com/P-p-H-d/mlib.git
#	GIT_TAG V0.8.0
#)
# We use swisstables for type traits as well
#FetchContent_Declare(
#	swisstable
#	GIT_REPOSITORY https://github.com/google/cwisstable.git
#	GIT_TAG 6de0e5f
#)
#FetchContent_MakeAvailable(swisstable)
#FetchContent_MakeAvailable(mlib)

file(GLOB SRC_FILES CONFIGURE_DEPENDS nil/*.c)

add_library(nil STATIC ${SRC_FILES})
target_include_directories(nil PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
#target_compile_options(nil PRIVATE --header-filter=internal/*.h)
#target_link_options(nil PUBLIC "-T${CMAKE_SOURCE_DIR}/nil.ld")
#target_link_options(nil PUBLIC "-T/d/_dev/_lib/c/cnil/nil.ld")
#target_link_options(nil INTERFACE
#	$<$<LINK_LANG_AND_ID:C,Clang,GNU>: -Wl,--whole-archive>
#	$<$<PLATFORM_ID:Darwin>: -Wl,-force_load>
#)
#target_link_options(nil INTERFACE -Wl,)

file(GLOB REFLECTOR_SRC_FILES CONFIGURE_DEPENDS nil_reflector/*.c)

find_package(Clang CONFIG REQUIRED)
add_executable(nil_reflector ${REFLECTOR_SRC_FILES})
target_include_directories(nil_reflector PRIVATE ${CLANG_INCLUDE_DIRS})
target_link_libraries(nil_reflector PRIVATE libclang)
target_link_libraries(nil_reflector PRIVATE nil)
#target_include_directories(nil_reflector PRIVATE .)

function(reflect_types target)
#	get_property(system_dirs TARGET ${target} PROPERTY INTERFACE_SYSTEM_INCLUDE_DIRECTORIES)
#	set(sys_include "$<TARGET_PROPERTY:${target},INTERFACE_SYSTEM_INCLUDE_DIRECTORIES>")
#	message("System include dirs: ${sys_include}")
#	get_property(dirs TARGET ${target} PROPERTY INCLUDE_DIRECTORIES)
#	foreach(dir ${dirs})
#		message(STATUS "dir='${dir}'")
#	endforeach()

	set(generated_sources "")
	foreach(header ${ARGN})
		get_filename_component(filename "${header}" NAME_WE)
		set(out "${CMAKE_CURRENT_BINARY_DIR}/${filename}.reflection.c")

		# If set wrong, this can wipe header files. I don't know why, I really hope this doesn't happen again.
		set(generator_flags
#			"$<LIST:TRANSFORM,$<TARGET_PROPERTY:${target},INTERFACE_SYSTEM_INCLUDE_DIRECTORIES>,PREPEND,-I>"
			"$<LIST:TRANSFORM,$<TARGET_PROPERTY:${target},INCLUDE_DIRECTORIES>,PREPEND,-I>"
			"$<LIST:TRANSFORM,$<TARGET_PROPERTY:${target},COMPILE_DEFINITIONS>,PREPEND,-D>"
			"$<TARGET_PROPERTY:${target},COMPILE_OPTIONS>"
		)

		add_custom_command(
			OUTPUT "${out}"
			COMMAND $<TARGET_FILE:nil_reflector> -std=c${CMAKE_C_STANDARD} -w ${generator_flags} "${header}" "${out}"
			DEPENDS "${header}" nil_reflector
			COMMENT "Reflecting ${header} -> ${out}"
			COMMAND_EXPAND_LISTS
			VERBATIM
		)

		list(APPEND generated_sources "${out}")
	endforeach()

	target_sources(${target} PRIVATE ${generated_sources})

	# TODO: Should we have this? I think this stops all attribute-related warnings, not just unused annotations?
	# TODO: Putting this here causes the generators to break. What the fuck.
#	target_compile_options(${target} PRIVATE -Wno-attributes)
#	target_link_options(${target} PRIVATE "-Wl,--whole-archive")
endfunction()

if(${NIL_TESTS})
	include(FetchContent)
	set(FETCHCONTENT_QUIET FALSE)
	FetchContent_Declare(
		cunit
		GIT_REPOSITORY https://github.com/tayne3/cunit.git
		GIT_TAG v0.2.0
	)
	FetchContent_MakeAvailable(cunit)

	file(GLOB TEST_SRC_FILES CONFIGURE_DEPENDS nil_tests/*.c)
	add_executable(nil_tests ${TEST_SRC_FILES})

	file(GLOB TEST_HEADER_FILES CONFIGURE_DEPENDS nil_tests/*.h)
	reflect_types(nil_tests ${TEST_HEADER_FILES})

	target_compile_options(nil_tests PRIVATE -fsanitize=address,undefined)
	target_link_options(nil_tests PRIVATE -fsanitize=address,undefined)

	target_link_libraries(nil_tests PRIVATE nil)
	target_link_libraries(nil_tests PRIVATE cunit::cunit)

	enable_testing()
	add_test(NAME nil_tests COMMAND nil_tests)
endif()

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
