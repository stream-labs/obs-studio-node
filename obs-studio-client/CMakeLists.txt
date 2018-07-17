CMAKE_MINIMUM_REQUIRED(VERSION 3.1)
PROJECT(obs-studio-client VERSION 1.0.0)

#############################
# CMake Settings
#############################

SET(NODEJS_URL "https://atom.io/download/atom-shell" CACHE STRING "Node.JS URL")
SET(NODEJS_NAME "iojs" CACHE STRING "Node.JS Name")
SET(NODEJS_VERSION "v2.0.4" CACHE STRING "Node.JS Version")

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
	"${PROJECT_SOURCE_DIR}/source/nodeobs_autoconfig.cpp" "${PROJECT_SOURCE_DIR}/source/nodeobs_autoconfig.hpp"
	"${PROJECT_SOURCE_DIR}/source/main.cpp"
	"${PROJECT_SOURCE_DIR}/source/volmeter.cpp" "${PROJECT_SOURCE_DIR}/source/volmeter.hpp"
	"${PROJECT_SOURCE_DIR}/source/video.cpp" "${PROJECT_SOURCE_DIR}/source/video.hpp"
)
SET(PROJECT_LIBRARIES
	${NODEJS_LIBRARIES}
	lib-streamlabs-ipc
)
SET(PROJECT_INCLUDE_PATHS
	${NODEJS_INCLUDE_DIRS}
	"${CMAKE_SOURCE_DIR}/source/"
	"${CMAKE_SOURCE_DIR}/lib-streamlabs-ipc/include"
	"${PROJECT_SOURCE_DIR}/../lib-streamlabs-ipc/third-party/lib-datalane/"
)
SET(PROJECT_DATA
	"resources/"
)
ADD_DEFINITIONS(-DBUILDING_NODE_EXTENSION)

IF(WIN32)
	# windows.h
	add_definitions(-DWIN32_LEAN_AND_MEAN)
	add_definitions(-DNOGPICAPMASKS)
	add_definitions(-DNOVIRTUALKEYCODES)
	add_definitions(-DNOWINMESSAGES)
	#add_definitions(-DNOWINSTYLES)
	add_definitions(-DNOSYSMETRICS)
	add_definitions(-DNOMENUS)
	add_definitions(-DNOICONS)
	add_definitions(-DNOKEYSTATES)
	add_definitions(-DNOSYSCOMMANDS)
	add_definitions(-DNORASTEROPS)
	add_definitions(-DNOSHOWWINDOW)
	add_definitions(-DNOATOM)
	add_definitions(-DNOCLIPBOARD)
	add_definitions(-DNOCOLOR)
	add_definitions(-DNOCTLMGR)
	add_definitions(-DNODRAWTEXT)
	add_definitions(-DNOGDI)
	add_definitions(-DNOKERNEL)
	#add_definitions(-DNOUSER)
	#add_definitions(-DNONLS)
	add_definitions(-DNOMB)
	add_definitions(-DNOMEMMGR)
	add_definitions(-DNOMETAFILE)
	add_definitions(-DNOMINMAX)
	#add_definitions(-DNOMSG)
	add_definitions(-DNOOPENFILE)
	add_definitions(-DNOSCROLL)
	add_definitions(-DNOSERVICE)
	add_definitions(-DNOSOUND)
	add_definitions(-DNOTEXTMETRIC)
	add_definitions(-DNOWH)
	#add_definitions(-DNOWINOFFSETS)
	add_definitions(-DNOCOMM)
	add_definitions(-DNOKANJI)
	add_definitions(-DNOHELP)
	add_definitions(-DNOPROFILER)
	add_definitions(-DNODEFERWINDOWPOS)
	add_definitions(-DNOMCX)
	add_definitions(-DNOIME)
	add_definitions(-DNOMDI)
ENDIF()

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