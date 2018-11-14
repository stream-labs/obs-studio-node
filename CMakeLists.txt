cmake_minimum_required(VERSION 3.0.0 FATAL_ERROR)

set(VERSION_FULL 0.3.21)
set(VERSION_BUILD 0)

find_package(Git)

if(EXISTS "${CMAKE_SOURCE_DIR}/.git" AND Git_FOUND)
	execute_process(
		COMMAND "${GIT_EXECUTABLE}" rev-list --count --topo-order v${VERSION_FULL}..HEAD
		WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}
		OUTPUT_STRIP_TRAILING_WHITESPACE
		ERROR_STRIP_TRAILING_WHITESPACE
		ERROR_QUIET
	)
endif()

project(obs-studio-node VERSION ${VERSION_FULL}.${VERSION_BUILD})
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake")

# CppCheck
include(cppcheck)

cppcheck(EXCLUDE "${PROJECT_SOURCE_DIR}/node_modules")

if(WIN32)
	# Resource VersionInfo
	set(PROJECT_PRODUCT_NAME "OBS Studio for Node.JS")
	set(PROJECT_COMPANY_NAME "Streamlabs")
	set(PROJECT_COPYRIGHT "Streamlabs © 2017 - 2018")
	set(PROJECT_LEGAL_TRADEMARKS_1 "")
	set(PROJECT_LEGAL_TRADEMARKS_2 "")

	configure_file(
		${PROJECT_SOURCE_DIR}/cmake/version.rc.in
		${PROJECT_BINARY_DIR}/version.rc
		@ONLY
	)

	# ToDo: Figure out a better way to tell the test setup where to find our binaries.
	configure_file(
		${PROJECT_SOURCE_DIR}/cmake/obs.js.in
		${PROJECT_SOURCE_DIR}/tests/helpers/obs.js
		@ONLY
	)
endif()

add_subdirectory(lib-streamlabs-ipc)
add_subdirectory(obs-studio-client)
add_subdirectory(obs-studio-server)
