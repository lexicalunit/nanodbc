add_custom_target(examples DEPENDS examples)
set(examples northwind usage rowset_iteration table_schema)

include_directories(${CMAKE_SOURCE_DIR} ${CMAKE_SOURCE_DIR}/src ${ODBC_INCLUDE_DIR})
link_directories(${CMAKE_BINARY_DIR}/lib)

foreach(config "ansi" "unicode")
	foreach(example ${examples})
		add_executable(example_${example}_${config} ${example}.cpp example_unicode_utils.h)
		if (${config} STREQUAL "ansi")
			target_link_libraries(example_${example}_${config} nanodbca_shared)
		else()
			target_link_libraries(example_${example}_${config} nanodbcu_shared)
		endif()
		add_dependencies(examples example_${example}_${config})
	endforeach()
endforeach()

set(example_empty ${CMAKE_CURRENT_SOURCE_DIR}/empty.cpp)
if(NOT EXISTS ${example_empty}) # Behavior is well-defined only for full paths.
	configure_file(${example_empty}.in ${example_empty} COPYONLY)
endif()

add_executable(example_empty_ansi ${example_empty} example_unicode_utils.h)
target_link_libraries(example_empty_ansi nanodbca_shared)
add_dependencies(examples example_empty_ansi)

add_executable(example_empty_unicode ${example_empty} example_unicode_utils.h)
target_link_libraries(example_empty_unicode nanodbcu_shared)
set_target_properties(example_empty_unicode PROPERTIES COMPILE_DEFINITIONS NANODBC_USE_UNICODE)
add_dependencies(examples example_empty_unicode)
