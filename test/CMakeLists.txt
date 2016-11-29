cmake_minimum_required(VERSION 2.8.7)
include(ExternalProject)

if(CMAKE_CXX_COMPILER_ID STREQUAL "Clang" OR CMAKE_COMPILER_IS_GNUCXX)
	# Workaround for a Travis-CI + Catch build issue: `error: ignoring #pragma gcc diagnostic`
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-unknown-pragmas")
endif()

find_package(Git REQUIRED)
ExternalProject_Add(
	catch
	PREFIX ${CMAKE_BINARY_DIR}/catch
	GIT_REPOSITORY https://github.com/philsquared/Catch.git
	TIMEOUT 10
	UPDATE_COMMAND ${GIT_EXECUTABLE} pull
	CONFIGURE_COMMAND ""
	BUILD_COMMAND ""
	INSTALL_COMMAND ""
	LOG_DOWNLOAD ON)
ExternalProject_Get_Property(catch source_dir)
set(CATCH_INCLUDE_DIR ${source_dir}/include CACHE INTERNAL "Path to include folder for Catch")

include_directories(${CMAKE_SOURCE_DIR} ${CMAKE_SOURCE_DIR}/src ${CATCH_INCLUDE_DIR} ${ODBC_INCLUDE_DIR})
link_directories(${CMAKE_BINARY_DIR}/lib)
file(GLOB headers *.h *.hpp)
add_custom_target(tests DEPENDS tests catch)
set(test_list mssql mysql odbc postgresql sqlite vertica)
foreach(test_item ${test_list})
    foreach(config "ansi" "unicode")
		set(test_name ${test_item}_${config}_tests)
		add_executable(${test_name} main.cpp ${test_item}_test.cpp ${headers})
		add_dependencies(${test_name} catch)
		if (${config} STREQUAL "ansi")
			target_link_libraries(${test_name} nanodbca_shared)
		else()
			target_link_libraries(${test_name} nanodbcu_shared)
            set_target_properties(${test_name} PROPERTIES COMPILE_DEFINITIONS NANODBC_USE_UNICODE)
		endif()
		add_test(NAME ${test_name} COMMAND ${test_name})
		add_custom_target(${test_item}_${config}_test
			COMMAND ${CMAKE_CTEST_COMMAND} --force-new-ctest-process --output-on-failure -R ${test_name})
		add_custom_target(${test_item}_${config}_check
			COMMAND ${CMAKE_CTEST_COMMAND} --force-new-ctest-process --output-on-failure -R ${test_name}
			DEPENDS ${test_name})
		add_dependencies(tests ${test_name})
	endforeach()
    add_custom_target(${test_item}_check DEPENDS ${test_item}_ansi_test ${test_item}_unicode_test)
    add_custom_target(${test_item}_test DEPENDS ${test_item}_ansi_test ${test_item}_unicode_test)
endforeach()
