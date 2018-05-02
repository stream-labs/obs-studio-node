CMAKE_MINIMUM_REQUIRED(VERSION 3.1)
PROJECT(obs-studio-client VERSION 1.0.0)

#############################
# CMake Settings
#############################

SET(NODEJS_URL "https://atom.io/download/atom-shell" CACHE STRING "Node.JS URL")
SET(NODEJS_NAME "iojs" CACHE STRING "Node.JS Name")
SET(NODEJS_VERSION "v1.8.3" CACHE STRING "Node.JS Version")

#############################
# Node.JS Settings
#############################
include(NodeJS)

nodejs_init()

#############################
# Source, Libraries & Directories
#############################
SET(PROJECT_SOURCE 
	"${CMAKE_SOURCE_DIR}/source/error.hpp"
	"${CMAKE_SOURCE_DIR}/source/obs-property.hpp" "${CMAKE_SOURCE_DIR}/source/obs-property.cpp"

	"${PROJECT_SOURCE_DIR}/source/shared.cpp" "${PROJECT_SOURCE_DIR}/source/shared.hpp"
	"${PROJECT_SOURCE_DIR}/source/utility.cpp" "${PROJECT_SOURCE_DIR}/source/utility.hpp"
	"${PROJECT_SOURCE_DIR}/source/utility-v8.cpp" "${PROJECT_SOURCE_DIR}/source/utility-v8.hpp"
	"${PROJECT_SOURCE_DIR}/source/controller.cpp" "${PROJECT_SOURCE_DIR}/source/controller.hpp"
	"${PROJECT_SOURCE_DIR}/source/fader.cpp" "${PROJECT_SOURCE_DIR}/source/fader.hpp"
	"${PROJECT_SOURCE_DIR}/source/global.cpp" "${PROJECT_SOURCE_DIR}/source/global.hpp"
	"${PROJECT_SOURCE_DIR}/source/input.cpp" "${PROJECT_SOURCE_DIR}/source/input.hpp"
	"${PROJECT_SOURCE_DIR}/source/isource.cpp" "${PROJECT_SOURCE_DIR}/source/isource.hpp"
	"${PROJECT_SOURCE_DIR}/source/properties.cpp" "${PROJECT_SOURCE_DIR}/source/properties.hpp"
	"${PROJECT_SOURCE_DIR}/source/filter.cpp" "${PROJECT_SOURCE_DIR}/source/filter.hpp"
	"${PROJECT_SOURCE_DIR}/source/transition.cpp" "${PROJECT_SOURCE_DIR}/source/transition.hpp"
	"${PROJECT_SOURCE_DIR}/source/scene.cpp" "${PROJECT_SOURCE_DIR}/source/scene.hpp"
	"${PROJECT_SOURCE_DIR}/source/sceneitem.cpp" "${PROJECT_SOURCE_DIR}/source/sceneitem.hpp"
	"${PROJECT_SOURCE_DIR}/source/nodeobs_api.cpp" "${PROJECT_SOURCE_DIR}/source/nodeobs_api.hpp"
	"${PROJECT_SOURCE_DIR}/source/nodeobs_service.cpp" "${PROJECT_SOURCE_DIR}/source/nodeobs_service.hpp"
	"${PROJECT_SOURCE_DIR}/source/nodeobs_display.cpp" "${PROJECT_SOURCE_DIR}/source/nodeobs_display.hpp"
	"${PROJECT_SOURCE_DIR}/source/nodeobs_settings.cpp" "${PROJECT_SOURCE_DIR}/source/nodeobs_settings.hpp"
	"${PROJECT_SOURCE_DIR}/source/main.cpp"
	"${PROJECT_SOURCE_DIR}/source/volmeter.cpp" "${PROJECT_SOURCE_DIR}/source/volmeter.hpp"
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
INSTALL(TARGETS obs-studio-client 
	RUNTIME DESTINATION "./" COMPONENT Runtime
	LIBRARY DESTINATION "./" COMPONENT Runtime
)
INSTALL(FILES $<TARGET_PDB_FILE:obs-studio-client> DESTINATION ./ OPTIONAL)

# JavaScript
INSTALL(
	DIRECTORY "${PROJECT_SOURCE_DIR}/../js/"
	DESTINATION "./"
	PATTERN "*.json" EXCLUDE
)
INSTALL(
	FILES "${CMAKE_SOURCE_DIR}/package.json"
	DESTINATION "./"
)