PROJECT(obs_studio_client VERSION ${obs-studio-node_VERSION})
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

SET(NODEJS_URL "https://atom.io/download/atom-shell" CACHE STRING "Node.JS URL")
SET(NODEJS_NAME "iojs" CACHE STRING "Node.JS Name")
SET(NODEJS_VERSION "v10.4.2" CACHE STRING "Node.JS Version")

if(WIN32)
	# Resource VersionInfo
	set(PROJECT_PRODUCT_NAME "OBS Studio for Node.JS (Server)")
	set(PROJECT_COPYRIGHT "Streamlabs © 2017 - 2018")

	configure_file(
		${obs-studio-node_SOURCE_DIR}/cmake/version.rc.in
		${PROJECT_BINARY_DIR}/version.rc
		@ONLY
	)
endif()

include(NodeJS)

set(STREAMLABS_BUILD ON CACHE BOOL "Whether to build for streamlabs-obs")

nodejs_init()

if (APPLE)
	find_library(COREFOUNDATION CoreFoundation)
	find_library(COCOA Cocoa)
	find_library(AVFOUNDATION AVFoundation)
	find_library(SECURITY_FRAMEWORK Security)
	find_library(FOUNDATION Foundation)
	find_library(IOSURF IOSurface)
	find_library(GLKIT GLKit)
	find_library(IOKit IOKit)
	find_library(SECURITY_LIBRARY Security)
	find_library(BSM_LIBRARY bsm)

	include_directories(
		${COREFOUNDATION}
		${COCOA}
		${AVFOUNDATION}
		${Security}
		${Foundation}
	)
endif ()

############################
# Setup crashpad binaries
############################

if(WIN32)
	download_project(
		PROJ crashpad
		URL https://obsstudionodes3.streamlabs.com/crashpad/crashpad-release-1.0.21-win-x64.tar.gz
		UPDATE_DISCONNECTED 1
	)
elseif(APPLE)
	download_project(
		PROJ crashpad
		URL https://obsstudionodes3.streamlabs.com/crashpad/crashpad-release-1.0.21-osx.zip
		UPDATE_DISCONNECTED 1
	)
endif()

ExternalProject_Add(
	crashpad_ep
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

# Getting LIBOBS_VERSION from azure script
file(STRINGS ../azure-pipelines.yml VERSION_LINE REGEX "LibOBSVersion")
string(REGEX MATCH "[^: ]*$" CURRENT_VERSION ${VERSION_LINE})

set(
	LIBOBS_VERSION
	${CURRENT_VERSION}
	CACHE STRING "Version of libobs being used"
)

set(
	LIBOBS_BUILD_TYPE
	"release"
	CACHE STRING "Build type of libobs artifact"
)

if(WIN32)
	set(LIBOBS_PLATFORM "windows64")
endif()
if(APPLE)
	set(LIBOBS_PLATFORM "osx")
endif()

string(
	CONCAT OSN_DEFAULT_LIBOBS_URL
	"https://obsstudios3.streamlabs.com/"
	"libobs-${LIBOBS_PLATFORM}-${LIBOBS_BUILD_TYPE}-${LIBOBS_VERSION}.7z"
)

set(
	OSN_LIBOBS_URL
	${OSN_DEFAULT_LIBOBS_URL}
	CACHE STRING "Location of libobs archive to base on"
)

download_project(
	PROJ libobs
	URL ${OSN_LIBOBS_URL}
	UPDATE_DISCONNECTED 1
)

include("${libobs_SOURCE_DIR}/cmake/LibObs/LibObsConfig.cmake")

SET(osn-client_SOURCES
	"${CMAKE_SOURCE_DIR}/source/error.hpp"
	"${CMAKE_SOURCE_DIR}/source/obs-property.hpp"
	"${CMAKE_SOURCE_DIR}/source/obs-property.cpp"

	"source/shared.cpp"
	"source/shared.hpp"
	"source/utility.cpp"
	"source/utility.hpp"
	"source/utility-v8.cpp"
	"source/utility-v8.hpp"
	"source/data-value.cpp"
	"source/data-value.hpp"
	"source/fader.cpp"
	"source/fader.hpp"
	"source/global.cpp"
	"source/global.hpp"
	"source/input.cpp"
	"source/input.hpp"
	"source/isource.cpp"
	"source/isource.hpp"
	"source/properties.cpp"
	"source/properties.hpp"
	"source/filter.cpp"
	"source/filter.hpp"
	"source/transition.cpp"
	"source/transition.hpp"
	"source/scene.cpp"
	"source/scene.hpp"
	"source/sceneitem.cpp"
	"source/sceneitem.hpp"
	"source/nodeobs_api.cpp"
	"source/nodeobs_api.hpp"
	"source/nodeobs_service.cpp"
	"source/nodeobs_service.hpp"
	"source/nodeobs_display.cpp"
	"source/nodeobs_display.hpp"
	"source/nodeobs_settings.cpp"
	"source/nodeobs_settings.hpp"
	"source/nodeobs_autoconfig.cpp"
	"source/nodeobs_autoconfig.hpp"
	"source/main.cpp"
	"source/volmeter.cpp"
	"source/volmeter.hpp"
	"source/video.cpp"
	"source/video.hpp"
	"source/module.cpp"
	"source/module.hpp"
	"source/cache-manager.hpp"
	"source/cache-manager.cpp"

	###### callback-manager ######
	"source/callback-manager.cpp"
	"source/callback-manager.hpp"

	###### obs-studio-server ######
	# "source/server//main.cpp"
	"source/server//shared-server.cpp"
	"source/server//shared-server.hpp"
	"source/server//utility-server.cpp"
	"source/server//utility-server.hpp"
	"source/server//osn-nodeobs.cpp"
	"source/server//osn-nodeobs.hpp"
	"source/server//osn-audio.cpp"
	"source/server//osn-audio.hpp"
	"source/server//osn-calldata.cpp"
	"source/server//osn-calldata.hpp"
	"source/server//osn-common.cpp"
	"source/server//osn-common.hpp"
	"source/server//osn-display.cpp"
	"source/server//osn-display.hpp"
	"source/server//osn-fader.cpp"
	"source/server//osn-fader.hpp"
	"source/server//osn-filter.cpp"
	"source/server//osn-filter.hpp"
	"source/server//osn-global.cpp"
	"source/server//osn-global.hpp"
	"source/server//osn-iencoder.cpp"
	"source/server//osn-iencoder.hpp"
	"source/server//osn-input.cpp"
	"source/server//osn-input.hpp"
	"source/server//osn-module.cpp"
	"source/server//osn-module.hpp"
	"source/server//osn-output.cpp"
	"source/server//osn-output.hpp"
	"source/server//osn-properties.cpp"
	"source/server//osn-properties.hpp"
	"source/server//osn-scene.cpp"
	"source/server//osn-scene.hpp"
	"source/server//osn-sceneitem.cpp"
	"source/server//osn-sceneitem.hpp"
	"source/server//osn-service.cpp"
	"source/server//osn-service.hpp"
	"source/server//osn-source.cpp"
	"source/server//osn-source.hpp"
	"source/server//osn-transition.cpp"
	"source/server//osn-transition.hpp"
	"source/server//osn-video.cpp"
	"source/server//osn-video.hpp"
	"source/server//osn-volmeter.cpp"
	"source/server//osn-volmeter.hpp"

	###### utlity graphics ######
	"source/server//gs-limits.h"
	"source/server//gs-vertex.h"
	"source/server//gs-vertex.cpp"
	"source/server//gs-vertexbuffer.h"
	"source/server//gs-vertexbuffer.cpp"

	###### node-obs ######
	"source/server//nodeobs_api-server.cpp"
	"source/server//nodeobs_api-server.h"
	"source/server//nodeobs_audio_encoders.cpp"
	"source/server//nodeobs_audio_encoders.h"
	"source/server//nodeobs_autoconfig-server.cpp"
	"source/server//nodeobs_autoconfig-server.h"
	"source/server//nodeobs_configManager.cpp"
	"source/server//nodeobs_configManager.hpp"
	"source/server//nodeobs_display-server.cpp"
	"source/server//nodeobs_display-server.h"
	"source/server//nodeobs_content.h"
	"source/server//nodeobs_common.cpp"
	"source/server//nodeobs_service-server.cpp"
	"source/server//nodeobs_service-server.h"
	"source/server//nodeobs_settings-server.cpp"
	"source/server//nodeobs_settings-server.h"
	"source/server//util-memory.cpp"
	"source/server//util-memory.h"

	###### crash-manager ######
	"source/server//util-crashmanager.cpp"
	"source/server//util-crashmanager.h"
	"source/server//util-metricsprovider.cpp"
	"source/server//util-metricsprovider.h"
	
	###### callback-manager ######
	"source/server//callback-manager.cpp"
	"source/server//callback-manager.h"

	###### memory-manager ######
	"source/server//memory-manager.cpp"
	"source/server//memory-manager.h"
)

if (APPLE)
	SET(osn-client-osx_SOURCES
		###### osx-util ######
		"${PROJECT_SOURCE_DIR}/source/util-osx.hpp"
		"${PROJECT_SOURCE_DIR}/source/util-osx.cpp"
		"${PROJECT_SOURCE_DIR}/source/util-osx-int.h"
		"${PROJECT_SOURCE_DIR}/source/util-osx-impl.h"
		"${PROJECT_SOURCE_DIR}/source/util-osx-impl.mm"
	)
	LIST(
		APPEND
		osn-client_SOURCES
		${osn-client-osx_SOURCES}
	)
endif ()

add_nodejs_module(
	obs_studio_client
	${osn-client_SOURCES}
)

set(PROJECT_INCLUDE_PATHS
	"${nlohmannjson_SOURCE_DIR}/single_include"
)

if(WIN32)
	target_sources(obs_studio_client PUBLIC ${PROJECT_BINARY_DIR}/version.rc)
	target_link_libraries(obs_studio_client ${LIBOBS_LIBRARIES} dwmapi.lib optimized crashpad StackWalker)
else ()
	target_link_libraries(obs_studio_client ${LIBOBS_LIBRARIES} ${COREFOUNDATION} ${COCOA} ${AVFOUNDATION} ${Security} ${Foundation} crashpad)
endif()

target_include_directories(
	obs_studio_client
	PUBLIC
		"${CMAKE_SOURCE_DIR}/source/"
		${PROJECT_INCLUDE_PATHS}
		${LIBOBS_INCLUDE_DIRS}
		"${stackwalker_SOURCE_DIR}/Main/StackWalker"
		"${nlohmannjson_SOURCE_DIR}/single_include"
)

# Include N-API wrappers
execute_process(COMMAND node -p "require('node-addon-api').include"
        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
        OUTPUT_VARIABLE NODE_ADDON_API_DIR
        )
string(REPLACE "\n" "" NODE_ADDON_API_DIR ${NODE_ADDON_API_DIR})
string(REPLACE "\"" "" NODE_ADDON_API_DIR ${NODE_ADDON_API_DIR})
target_include_directories(${PROJECT_NAME} PRIVATE ${NODE_ADDON_API_DIR})
target_link_libraries(${PROJECT_NAME} ${LIBOBS_LIBRARIES})

# Define NAPI_VERSION
add_definitions(-DNAPI_VERSION=4)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++17")
add_definitions(-std=c++17)

#Define the OSN_VERSION
add_compile_definitions(OSN_VERSION=\"$ENV{tagartifact}\") 
set_target_properties(
	obs_studio_client
	PROPERTIES
	VERSION ${PROJECT_VERSION}
	SOVERSION ${PROJECT_VERSION}
	PREFIX "" SUFFIX ".node"
)

target_compile_definitions(obs_studio_client PRIVATE BUILDING_NODE_EXTENSION)

if(WIN32)
	target_compile_definitions(
		obs_studio_client
		PRIVATE
			WIN32_LEAN_AND_MEAN
			NOMINMAX
			UNICODE
			_UNICODE
	)
endif()

IF( NOT CLANG_ANALYZE_CONFIG)
	cppcheck_add_project(${PROJECT_NAME})
ENDIF()

IF(WIN32 AND NOT CLANG_ANALYZE_CONFIG)
	install(FILES $<TARGET_PDB_FILE:obs_studio_client> DESTINATION ./ OPTIONAL)
ENDIF()
install(FILES "${CMAKE_SOURCE_DIR}/package.json" DESTINATION "./")

set(PROJECT_DATA "${PROJECT_SOURCE_DIR}/resources")

install(DIRECTORY ${PROJECT_DATA} DESTINATION "./" OPTIONAL USE_SOURCE_PERMISSIONS)
install(DIRECTORY ${crashpad_SOURCE_DIR}/bin/ DESTINATION "./" USE_SOURCE_PERMISSIONS)
install(DIRECTORY "${libobs_SOURCE_DIR}/data/" DESTINATION "./data" USE_SOURCE_PERMISSIONS)

if (WIN32)
	install(
		DIRECTORY "${libobs_SOURCE_DIR}/bin/64bit/"
		DESTINATION "./" PATTERN "*.lib" EXCLUDE
	)
else ()
	install(
		DIRECTORY "${libobs_SOURCE_DIR}/bin/"
		DESTINATION "./" USE_SOURCE_PERMISSIONS PATTERN "*.lib" EXCLUDE
	)
endif ()

if (WIN32)
install(
	DIRECTORY "${libobs_SOURCE_DIR}/obs-plugins/"
	DESTINATION "./obs-plugins"
)
else ()
install(
	DIRECTORY "${libobs_SOURCE_DIR}/obs-plugins/"
	DESTINATION "./obs-plugins"
	USE_SOURCE_PERMISSIONS
)
endif()

install(
	DIRECTORY "${CMAKE_SOURCE_DIR}/obs-studio-client/../js/"
	DESTINATION "./"
	PATTERN "*.json" EXCLUDE
)

install(
	TARGETS obs_studio_client
	RUNTIME DESTINATION "./" COMPONENT Runtime
	LIBRARY DESTINATION "./" COMPONENT Runtime
	ARCHIVE DESTINATION "./" COMPONENT Runtime	
)
