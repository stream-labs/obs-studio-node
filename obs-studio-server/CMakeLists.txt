CMAKE_MINIMUM_REQUIRED(VERSION 3.0)
PROJECT(obs-studio-server VERSION 0.0.1)

#############################
# CMake Settings
#############################

# Modules
SET(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake")
include(ExternalProject)
include(DownloadProject)

############################
# Setup crashpad binaries
############################
download_project(
	PROJ crashpad
	URL https://ci.appveyor.com/api/buildjobs/r1068pgmh2gv2x25/artifacts/win-x64.tar.gz
	UPDATE_DISCONNECTED 1
)

ExternalProject_Add(crashpad_ep	
	DOWNLOAD_COMMAND ""
	SOURCE_DIR "${crashpad_SOURCE_DIR}"
	INSTALL_COMMAND ""
	BUILD_COMMAND ""
	CONFIGURE_COMMAND ""
	BUILD_BYPRODUCTS 
		"<SOURCE_DIR>/lib/${CMAKE_STATIC_LIBRARY_PREFIX}base${CMAKE_STATIC_LIBRARY_SUFFIX}"
		"<SOURCE_DIR>/lib/${CMAKE_STATIC_LIBRARY_PREFIX}util${CMAKE_STATIC_LIBRARY_SUFFIX}"
		"<SOURCE_DIR>/lib/${CMAKE_STATIC_LIBRARY_PREFIX}client${CMAKE_STATIC_LIBRARY_SUFFIX}"
		"<SOURCE_DIR>/bin/crashpad_database_util${CMAKE_EXECUTABLE_SUFFIX}"
		"<SOURCE_DIR>/bin/crashpad_handler${CMAKE_EXECUTABLE_SUFFIX}"
		"<SOURCE_DIR>/bin/crashpad_http_upload${CMAKE_EXECUTABLE_SUFFIX}"
)

# Our crashpad artifacts assume a particular format
# <dir>\bin contains external processes
# <dir>\lib contains static libraries we need to link against
# <dir>\include contains the primary include path
# <dir>\include\third_party\mini_chromium contains chromium include files

add_library(crashpad_util STATIC IMPORTED)
add_library(crashpad_base STATIC IMPORTED)
add_library(crashpad_client STATIC IMPORTED)
add_executable(crashpad_handler IMPORTED)
add_executable(crashpad_database_util IMPORTED)
add_executable(crashpad_http_upload IMPORTED)
# From this, we get three total targets:
#   crashpad_base
#   crashpad_util
#   crashpad_client
# It's recommended to use util but not required as far as I know.

ExternalProject_Get_Property(crashpad_ep source_dir)

set_property(TARGET crashpad_base PROPERTY IMPORTED_LOCATION
	"${source_dir}/lib/${CMAKE_STATIC_LIBRARY_PREFIX}base${CMAKE_STATIC_LIBRARY_SUFFIX}")

set_property(TARGET crashpad_util PROPERTY IMPORTED_LOCATION
	"${source_dir}/lib/${CMAKE_STATIC_LIBRARY_PREFIX}util${CMAKE_STATIC_LIBRARY_SUFFIX}")

set_property(TARGET crashpad_client PROPERTY IMPORTED_LOCATION
	"${source_dir}/lib/${CMAKE_STATIC_LIBRARY_PREFIX}client${CMAKE_STATIC_LIBRARY_SUFFIX}")

set_property(TARGET crashpad_handler PROPERTY IMPORTED_LOCATION
	"${source_dir}/bin/crashpad_handler${CMAKE_EXECUTABLE_SUFFIX}")

set_property(TARGET crashpad_database_util PROPERTY IMPORTED_LOCATION
	"${source_dir}/bin/crashpad_database_util${CMAKE_EXECUTABLE_SUFFIX}")

set_property(TARGET crashpad_http_upload PROPERTY IMPORTED_LOCATION
	"${source_dir}/bin/crashpad_http_upload${CMAKE_EXECUTABLE_SUFFIX}")

target_include_directories(
	crashpad_base
	INTERFACE "${source_dir}/include/third_party/mini_chromium/mini_chromium"
)

target_include_directories(
	crashpad_client
	INTERFACE "${source_dir}/include"
)

add_library(crashpad INTERFACE)
target_link_libraries(
	crashpad
	INTERFACE
		crashpad_base
		crashpad_client
		crashpad_util
)

target_link_libraries(crashpad_util INTERFACE crashpad_client)

add_dependencies(crashpad_base crashpad_ep)
add_dependencies(crashpad_client crashpad_base)
add_dependencies(crashpad_util crashpad_client)

#############################
# Prerequisites
#############################

# OBS Studio (CPack Release)
download_project(
	PROJ libobs
	URL https://ci.appveyor.com/api/buildjobs/659km5wsgap2sr90/artifacts/windows-x86_64-dev.zip
	UPDATE_DISCONNECTED 1
)
INCLUDE("${libobs_SOURCE_DIR}/cmake/LibObs/LibObsConfig.cmake")

# OBS Studio (Signed Binaries)
download_project(
	PROJ libobs_signed
	URL https://github.com/obsproject/obs-studio/releases/download/21.1.2/OBS-Studio-21.1.2-Small.zip
	UPDATE_DISCONNECTED 1
)

#############################
# Source, Libraries & Directories
#############################
SET(PROJECT_SOURCE
	"${CMAKE_SOURCE_DIR}/source/error.hpp"
	"${CMAKE_SOURCE_DIR}/source/obs-property.hpp" "${CMAKE_SOURCE_DIR}/source/obs-property.cpp"
	
	###### OBS-STUDIO-NODE ######
	"${PROJECT_SOURCE_DIR}/source/main.cpp"
	"${PROJECT_SOURCE_DIR}/source/shared.cpp" "${PROJECT_SOURCE_DIR}/source/shared.hpp"
	"${PROJECT_SOURCE_DIR}/source/utility.cpp" "${PROJECT_SOURCE_DIR}/source/utility.hpp"	
	"${PROJECT_SOURCE_DIR}/source/osn-nodeobs.cpp" "${PROJECT_SOURCE_DIR}/source/osn-nodeobs.hpp"
	"${PROJECT_SOURCE_DIR}/source/osn-audio.cpp" "${PROJECT_SOURCE_DIR}/source/osn-audio.hpp"
	"${PROJECT_SOURCE_DIR}/source/osn-calldata.cpp" "${PROJECT_SOURCE_DIR}/source/osn-calldata.hpp"
	"${PROJECT_SOURCE_DIR}/source/osn-common.cpp" "${PROJECT_SOURCE_DIR}/source/osn-common.hpp"
	"${PROJECT_SOURCE_DIR}/source/osn-display.cpp" "${PROJECT_SOURCE_DIR}/source/osn-display.hpp"
	"${PROJECT_SOURCE_DIR}/source/osn-fader.cpp" "${PROJECT_SOURCE_DIR}/source/osn-fader.hpp"
	"${PROJECT_SOURCE_DIR}/source/osn-filter.cpp" "${PROJECT_SOURCE_DIR}/source/osn-filter.hpp"
	"${PROJECT_SOURCE_DIR}/source/osn-global.cpp" "${PROJECT_SOURCE_DIR}/source/osn-global.hpp"
	"${PROJECT_SOURCE_DIR}/source/osn-iencoder.cpp" "${PROJECT_SOURCE_DIR}/source/osn-iencoder.hpp"
	"${PROJECT_SOURCE_DIR}/source/osn-input.cpp" "${PROJECT_SOURCE_DIR}/source/osn-input.hpp"
	"${PROJECT_SOURCE_DIR}/source/osn-module.cpp" "${PROJECT_SOURCE_DIR}/source/osn-module.hpp"
	"${PROJECT_SOURCE_DIR}/source/osn-output.cpp" "${PROJECT_SOURCE_DIR}/source/osn-output.hpp"
	"${PROJECT_SOURCE_DIR}/source/osn-properties.cpp" "${PROJECT_SOURCE_DIR}/source/osn-properties.hpp"
	"${PROJECT_SOURCE_DIR}/source/osn-scene.cpp" "${PROJECT_SOURCE_DIR}/source/osn-scene.hpp"
	"${PROJECT_SOURCE_DIR}/source/osn-sceneitem.cpp" "${PROJECT_SOURCE_DIR}/source/osn-sceneitem.hpp"
	"${PROJECT_SOURCE_DIR}/source/osn-service.cpp" "${PROJECT_SOURCE_DIR}/source/osn-service.hpp"
	"${PROJECT_SOURCE_DIR}/source/osn-source.cpp" "${PROJECT_SOURCE_DIR}/source/osn-source.hpp"
	"${PROJECT_SOURCE_DIR}/source/osn-transition.cpp" "${PROJECT_SOURCE_DIR}/source/osn-transition.hpp"
	"${PROJECT_SOURCE_DIR}/source/osn-video.cpp" "${PROJECT_SOURCE_DIR}/source/osn-video.hpp"
	"${PROJECT_SOURCE_DIR}/source/osn-volmeter.cpp" "${PROJECT_SOURCE_DIR}/source/osn-volmeter.hpp"
		
	###### Graphics ######
	"${PROJECT_SOURCE_DIR}/source/gs-limits.h"
	"${PROJECT_SOURCE_DIR}/source/gs-vertex.h" "${PROJECT_SOURCE_DIR}/source/gs-vertex.cpp"
	"${PROJECT_SOURCE_DIR}/source/gs-vertexbuffer.h" "${PROJECT_SOURCE_DIR}/source/gs-vertexbuffer.cpp"
	
	###### NODE-OBS ######
	"${PROJECT_SOURCE_DIR}/source/nodeobs_api.cpp" "${PROJECT_SOURCE_DIR}/source/nodeobs_api.h"
	"${PROJECT_SOURCE_DIR}/source/nodeobs_audio_encoders.cpp" "${PROJECT_SOURCE_DIR}/source/nodeobs_audio_encoders.h"
	"${PROJECT_SOURCE_DIR}/source/nodeobs_autoconfig.cpp" "${PROJECT_SOURCE_DIR}/source/nodeobs_autoconfig.h"
	###### "${PROJECT_SOURCE_DIR}/source/nodeobs_async.h"
	"${PROJECT_SOURCE_DIR}/source/nodeobs_display.cpp" "${PROJECT_SOURCE_DIR}/source/nodeobs_display.h"
	"${PROJECT_SOURCE_DIR}/source/nodeobs_content.h" "${PROJECT_SOURCE_DIR}/source/nodeobs_common.cpp"	
	"${PROJECT_SOURCE_DIR}/source/nodeobs_service.cpp" "${PROJECT_SOURCE_DIR}/source/nodeobs_service.h"
	"${PROJECT_SOURCE_DIR}/source/nodeobs_settings.cpp" "${PROJECT_SOURCE_DIR}/source/nodeobs_settings.h"
	"${PROJECT_SOURCE_DIR}/source/util-memory.cpp" "${PROJECT_SOURCE_DIR}/source/util-memory.h"
)
SET(PROJECT_LIBRARIES
	lib-streamlabs-ipc
	${LIBOBS_LIBRARIES}
)
SET(PROJECT_INCLUDE_PATHS
	"${CMAKE_SOURCE_DIR}/source"
	"${PROJECT_SOURCE_DIR}/source"
	"${lib-streamlabs-ipc_SOURCE_DIR}/include"
	${LIBOBS_INCLUDE_DIRS}
)
SET(PROJECT_DATA
	"${PROJECT_SOURCE_DIR}/resources"
)

#############################
# Building
#############################
ADD_EXECUTABLE(obs-studio-server 
	${PROJECT_SOURCE}
)
TARGET_LINK_LIBRARIES(obs-studio-server
	${PROJECT_LIBRARIES}
	optimized crashpad
)
TARGET_INCLUDE_DIRECTORIES(obs-studio-server 
	PUBLIC ${PROJECT_INCLUDE_PATHS}
)

#############################
# Distribute
#############################
INSTALL(TARGETS obs-studio-server
	RUNTIME DESTINATION "./" COMPONENT Runtime
)
INSTALL(FILES $<TARGET_PDB_FILE:obs-studio-server> DESTINATION "./" OPTIONAL)
INSTALL(DIRECTORY ${PROJECT_DATA} DESTINATION "./" OPTIONAL)

# Crashpad
INSTALL(DIRECTORY ${crashpad_SOURCE_DIR}/bin/ DESTINATION "./")

# libobs
INSTALL(DIRECTORY "${libobs_SOURCE_DIR}/bin/64bit/" DESTINATION "./" PATTERN "*.lib" EXCLUDE)
INSTALL(DIRECTORY "${libobs_SOURCE_DIR}/data/" DESTINATION "./data")
INSTALL(DIRECTORY "${libobs_SOURCE_DIR}/obs-plugins/" DESTINATION "./obs-plugins")
INSTALL(DIRECTORY "${libobs_signed_SOURCE_DIR}/data/obs-plugins/win-capture/" DESTINATION "./data/obs-plugins/win-capture")
