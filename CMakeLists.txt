# TODO: Include https://github.com/taviso/swisstable?

cmake_minimum_required(VERSION 3.31)
project(cnil C)

set(CMAKE_C_STANDARD 23)
set(CMAKE_EXPORT_COMPILE_COMMANDS)

include(FetchContent)
set(FETCHCONTENT_QUIET FALSE)
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

		# If set wrong, this can wipe header files. I don't know why, I really hope this doesn't happen again.
		set(generator_flags
			"$$<LIST:TRANSFORM,$<TARGET_PROPERTY:${target},INCLUDE_DIRECTORIES>,PREPEND,-I>"
			"$$<LIST:TRANSFORM,$<TARGET_PROPERTY:${target},COMPILE_DEFINITIONS>,PREPEND,-D>"
			"$$<LIST:TRANSFORM,$<TARGET_PROPERTY:${target},COMPILE_OPTIONS>,>"
		)

		add_custom_command(
			OUTPUT "${out}"
			COMMAND $<TARGET_FILE:nil_reflector> -std=c23 -w ${generator_flags} "${header}" "${out}"
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
