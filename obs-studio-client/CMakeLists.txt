PROJECT(
	obs-studio-client
	VERSION ${obs-studio-node_VERSION_MAJOR}.${obs-studio-node_VERSION_MINOR}.${obs-studio-node_VERSION_PATCH}.${obs-studio-node_VERSION_TWEAK}
)

#############################
# CMake Settings
#############################

SET(NODEJS_URL "https://atom.io/download/atom-shell" CACHE STRING "Node.JS URL")
SET(NODEJS_NAME "iojs" CACHE STRING "Node.JS Name")
SET(NODEJS_VERSION "v2.0.4" CACHE STRING "Node.JS Version")

################################################################################
# Compiler
################################################################################

# MSVC Specific
IF(WIN32)
	# Resource VersionInfo
	SET(
		PROJECT_PRODUCT_NAME
		"OBS Studio for Node.JS (Server)"
	)
	SET(
		PROJECT_COPYRIGHT
		"Streamlabs © 2017 - 2018"
	)
	
	configure_file(
		${obs-studio-node_SOURCE_DIR}/cmake/version.rc.in
		${PROJECT_BINARY_DIR}/version.rc
		@ONLY
	)
ENDIF()

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

IF(WIN32)
	LIST(APPEND PROJECT_SOURCE
		${PROJECT_BINARY_DIR}/version.rc
	)
ENDIF()

#############################
# Building
#############################
ADD_LIBRARY(
	${PROJECT_NAME}
	MODULE
	${PROJECT_SOURCE}
)

TARGET_LINK_LIBRARIES(
	${PROJECT_NAME}
	${PROJECT_LIBRARIES}
)

TARGET_INCLUDE_DIRECTORIES(
	${PROJECT_NAME}
	PUBLIC ${PROJECT_INCLUDE_PATHS}
)

SET_TARGET_PROPERTIES(
	${PROJECT_NAME}
	PROPERTIES
	PREFIX ""
	SUFFIX ".node"
)

TARGET_COMPILE_DEFINITIONS(
	${PROJECT_NAME}
	PRIVATE
	-DUNICODE
	-D_UNICODE
)

IF(WIN32)
	SET_TARGET_PROPERTIES(
		${PROJECT_NAME}
		PROPERTIES
		VERSION ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}.${PROJECT_VERSION_PATCH}.${PROJECT_VERSION_TWEAK}
		SOVERSION ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}.${PROJECT_VERSION_PATCH}.${PROJECT_VERSION_TWEAK}
	)
ELSE()
	SET_TARGET_PROPERTIES(
		${PROJECT_NAME}
		PROPERTIES
		VERSION ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}.${PROJECT_VERSION_PATCH}.${PROJECT_VERSION_TWEAK}
		SOVERSION ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}.${PROJECT_VERSION_PATCH}.${PROJECT_VERSION_TWEAK}
	)
ENDIF()

target_compile_definitions(
	${PROJECT_NAME}
	PRIVATE BUILDING_NODE_EXTENSION
)

IF(WIN32)
	# windows.h
	target_compile_definitions(
		${PROJECT_NAME}
		PRIVATE WIN32_LEAN_AND_MEAN
		PRIVATE NOGPICAPMASKS
		PRIVATE NOVIRTUALKEYCODES
		PRIVATE NOWINMESSAGES
	#	PRIVATE NOWINSTYLES
		PRIVATE NOSYSMETRICS
		PRIVATE NOMENUS
		PRIVATE NOICONS
		PRIVATE NOKEYSTATES
		PRIVATE NOSYSCOMMANDS
		PRIVATE NORASTEROPS
		PRIVATE NOSHOWWINDOW
		PRIVATE NOATOM
		PRIVATE NOCLIPBOARD
		PRIVATE NOCOLOR
		PRIVATE NOCTLMGR
		PRIVATE NODRAWTEXT
		PRIVATE NOGDI
		PRIVATE NOKERNEL
	#	PRIVATE NOUSER
	#	PRIVATE NONLS
		PRIVATE NOMB
		PRIVATE NOMEMMGR
		PRIVATE NOMETAFILE
		PRIVATE NOMINMAX
	#	PRIVATE NOMSG
		PRIVATE NOOPENFILE
		PRIVATE NOSCROLL
		PRIVATE NOSERVICE
		PRIVATE NOSOUND
		PRIVATE NOTEXTMETRIC
		PRIVATE NOWH
	#	PRIVATE NOWINOFFSETS
		PRIVATE NOCOMM
		PRIVATE NOKANJI
		PRIVATE NOHELP
		PRIVATE NOPROFILER
		PRIVATE NODEFERWINDOWPOS
		PRIVATE NOMCX
		PRIVATE NOIME
		PRIVATE NOMDI
	)
ENDIF()

#############################
# Distribute
#############################
INSTALL(TARGETS ${PROJECT_NAME}
	RUNTIME DESTINATION "./" COMPONENT Runtime
	LIBRARY DESTINATION "./" COMPONENT Runtime
)
INSTALL(FILES $<TARGET_PDB_FILE:${PROJECT_NAME}> DESTINATION ./ OPTIONAL)

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