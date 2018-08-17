CMAKE_MINIMUM_REQUIRED(VERSION 3.1)

# Automatic Versioning
SET(VERSION_FULL 0.3.0)
SET(VERSION_BUILD 0)
IF(EXISTS "${CMAKE_SOURCE_DIR}/.git")
	SET(GIT_RESULT "")
	SET(GIT_OUTPUT "")
	EXECUTE_PROCESS(
		COMMAND git rev-list --count --topo-order v${VERSION_FULL}..HEAD
		WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
		RESULT_VARIABLE GIT_RESULT
		OUTPUT_VARIABLE GIT_OUTPUT
		OUTPUT_STRIP_TRAILING_WHITESPACE ERROR_STRIP_TRAILING_WHITESPACE ERROR_QUIET
	)
	IF(GIT_RESULT EQUAL 0)
		SET(VERSION_BUILD "${GIT_OUTPUT}")
	ENDIF()
ENDIF()

PROJECT(
	obs-studio-node
	VERSION ${VERSION_FULL}.${VERSION_BUILD}
	DESCRIPTION "An obs-studio C/C++ wrapper for Node.JS"
	HOMEPAGE_URL "https://streamlabs.com/streamlabs-obs"
)

################################################################################
# CMake
################################################################################
MESSAGE(STATUS "OBS Studio for Node.JS Version ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}.${PROJECT_VERSION_PATCH}.${PROJECT_VERSION_TWEAK}")

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
