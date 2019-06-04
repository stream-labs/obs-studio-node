cmake_minimum_required(VERSION 3.0.0 FATAL_ERROR)

IF(CLANG_ANALYZE_CONFIG)
	set(CMAKE_C_COMPILER_WORKS 1)
	set(CMAKE_CXX_COMPILER_WORKS 1)
	
	SET (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Xclang -fexceptions -Xclang -fcxx-exceptions")
ENDIF()

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
IF( NOT CLANG_ANALYZE_CONFIG)
	include(cppcheck)

	cppcheck(EXCLUDE "${PROJECT_SOURCE_DIR}/node_modules")
ENDIF()

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

	configure_file(
		${PROJECT_SOURCE_DIR}/cmake/index.ts.in
		${PROJECT_SOURCE_DIR}/tests/osn-tests/osn/index.ts
		@ONLY
	)
endif()

############################
# Setup global dependencies
############################

include(ExternalProject)
include(DownloadProject)
include(FetchContent)

if(MSVC)
	add_definitions(/MP)
	add_definitions(/debug:fastlink)
endif()

# Override a test flag
set(StackWalker_DISABLE_TESTS true CACHE BOOL "" FORCE)

# StackWalker (Callstack rewind
FetchContent_Declare(
  stackwalker
  GIT_REPOSITORY https://github.com/stream-labs/StackWalker
)

# Nlohmann JSON (modern JSON for C++)
FetchContent_Declare(
  nlohmannjson
  GIT_REPOSITORY https://github.com/nlohmann/json
)

FetchContent_GetProperties(stackwalker)
if(NOT stackwalker_POPULATED)
  FetchContent_Populate(stackwalker)
  add_subdirectory(${stackwalker_SOURCE_DIR} ${stackwalker_BINARY_DIR})
endif()

FetchContent_GetProperties(nlohmannjson)
if(NOT nlohmannjson_POPULATED)
  FetchContent_Populate(nlohmannjson)
endif()

add_subdirectory(lib-streamlabs-ipc)
add_subdirectory(obs-studio-client)
add_subdirectory(obs-studio-server)

include(CPack)