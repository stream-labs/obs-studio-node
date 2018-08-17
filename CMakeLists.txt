CMAKE_MINIMUM_REQUIRED(VERSION 3.1)
PROJECT(obs-studio-node VERSION 0.3.0.0)

################################################################################
# CMake / Compiler
################################################################################

# Retrieve Tweak version from git.
IF(EXISTS "${PROJECT_SOURCE_DIR}/.git")
	SET(GIT_RESULT "")
	SET(GIT_OUTPUT "")
	EXECUTE_PROCESS(
		COMMAND git rev-list --count --topo-order v${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}.${PROJECT_VERSION_PATCH}..HEAD
		WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}
		RESULT_VARIABLE GIT_RESULT
		OUTPUT_VARIABLE GIT_OUTPUT
		OUTPUT_STRIP_TRAILING_WHITESPACE ERROR_STRIP_TRAILING_WHITESPACE ERROR_QUIET
	)
	IF(GIT_RESULT EQUAL 0)
		SET(PROJECT_VERSION_TWEAK ${GIT_OUTPUT})
		MESSAGE(STATUS "Set Version to ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}.${PROJECT_VERSION_PATCH}.${PROJECT_VERSION_TWEAK}")
	ENDIF()
ELSE()
	SET(PROJECT_VERSION_TWEAK 0)
	MESSAGE(STATUS "Set Version to ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}.${PROJECT_VERSION_PATCH}.${PROJECT_VERSION_TWEAK}")
ENDIF()

# Set Module Path
SET(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake")

#############################
# Configuration
#############################

# Installation
SET(CMAKE_INSTALL_PREFIX  "${CMAKE_SOURCE_DIR}/distribute" CACHE PATH "Path to install files to")

#############################
# Projects
#############################

ADD_SUBDIRECTORY(lib-streamlabs-ipc)
ADD_SUBDIRECTORY(obs-studio-client)
ADD_SUBDIRECTORY(obs-studio-server)
