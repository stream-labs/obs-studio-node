CMAKE_MINIMUM_REQUIRED(VERSION 3.1)
PROJECT(obs-studio-client VERSION 1.0.0)

#############################
# CMake Settings
#############################

SET(NODEJS_CUSTOM_BUILD FALSE CACHE BOOL "Build with adjusted Node.JS runtime")
SET(NODEJS_CUSTOM_URL "https://atom.io/download/atom-shell" CACHE STRING "Node.JS URL")
SET(NODEJS_CUSTOM_NAME "iojs" CACHE STRING "Node.JS Name")
SET(NODEJS_CUSTOM_VERSION "v1.7.12" CACHE STRING "Node.JS Version")

#############################
# Node.JS Settings
#############################
include(NodeJS)

IF (NODEJS_CUSTOM_BUILD)
	nodejs_init(
		URL "${NODEJS_CUSTOM_URL}"
		NAME "${NODEJS_CUSTOM_NAME}"
		VERSION "${NODEJS_CUSTOM_VERSION}"
	)
ELSE()
	nodejs_init()
ENDIF()


#############################
# Source, Libraries & Directories
#############################
SET(PROJECT_SOURCE 
	"${CMAKE_SOURCE_DIR}/source/error.hpp"
	"${PROJECT_SOURCE_DIR}/source/main.cpp"
	"${PROJECT_SOURCE_DIR}/source/shared.cpp" "${PROJECT_SOURCE_DIR}/source/shared.hpp"
	"${PROJECT_SOURCE_DIR}/source/utility.cpp" "${PROJECT_SOURCE_DIR}/source/utility.hpp"
	"${PROJECT_SOURCE_DIR}/source/utility-v8.cpp" "${PROJECT_SOURCE_DIR}/source/utility-v8.hpp"
	"${PROJECT_SOURCE_DIR}/source/controller.cpp" "${PROJECT_SOURCE_DIR}/source/controller.hpp"
	"${PROJECT_SOURCE_DIR}/source/isource.cpp" "${PROJECT_SOURCE_DIR}/source/isource.hpp"
)
SET(PROJECT_LIBRARIES
	${NODEJS_LIBRARIES}
	lib-streamlabs-ipc
)
SET(PROJECT_INCLUDE_PATHS
	${NODEJS_INCLUDE_DIRS}
	"${CMAKE_SOURCE_DIR}/source/"
	"${CMAKE_SOURCE_DIR}/lib-streamlabs-ipc/include"
)
SET(PROJECT_DATA
	"resources/"
)
ADD_DEFINITIONS(-DBUILDING_NODE_EXTENSION)

#############################
# Building
#############################
ADD_LIBRARY(obs-studio-client SHARED ${PROJECT_SOURCE})
TARGET_LINK_LIBRARIES(obs-studio-client ${PROJECT_LIBRARIES})
TARGET_INCLUDE_DIRECTORIES(obs-studio-client PUBLIC ${PROJECT_INCLUDE_PATHS})
SET_TARGET_PROPERTIES(obs-studio-client PROPERTIES PREFIX "" SUFFIX ".node")

#############################
# Distribute
#############################
INSTALL(TARGETS obs-studio-client DESTINATION "${INSTALL_DIR}")
INSTALL(FILES $<TARGET_PDB_FILE:obs-studio-client> DESTINATION "${INSTALL_DIR}" OPTIONAL)
