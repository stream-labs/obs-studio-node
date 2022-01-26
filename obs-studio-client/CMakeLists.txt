PROJECT(obs_studio_client VERSION ${obs-studio-node_VERSION})
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

SET(NODEJS_URL "https://atom.io/download/atom-shell" CACHE STRING "Node.JS URL")
SET(NODEJS_NAME "iojs" CACHE STRING "Node.JS Name")
SET(NODEJS_VERSION "v13.4.0" CACHE STRING "Node.JS Version")

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

download_project(
	PROJ thread-pool
	GIT_REPOSITORY https://github.com/bshoshany/thread-pool.git
	GIT_TAG v2.0.0
	UPDATE_DISCONNECTED 1
)

SET(osn-client_SOURCES
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

	###### obs-wrapper ######
	"source/obs-wrapper/obs-shared.cpp"
	"source/obs-wrapper/obs-shared.hpp"
	"source/obs-wrapper/obs-utility.cpp"
	"source/obs-wrapper/obs-utility.hpp"
	"source/obs-wrapper/osn-calldata.cpp"
	"source/obs-wrapper/osn-calldata.hpp"
	"source/obs-wrapper/osn-display.cpp"
	"source/obs-wrapper/osn-display.hpp"
	"source/obs-wrapper/osn-fader.cpp"
	"source/obs-wrapper/osn-fader.hpp"
	"source/obs-wrapper/osn-filter.cpp"
	"source/obs-wrapper/osn-filter.hpp"
	"source/obs-wrapper/osn-global.cpp"
	"source/obs-wrapper/osn-global.hpp"
	"source/obs-wrapper/osn-input.cpp"
	"source/obs-wrapper/osn-input.hpp"
	"source/obs-wrapper/osn-module.cpp"
	"source/obs-wrapper/osn-module.hpp"
	"source/obs-wrapper/osn-output.cpp"
	"source/obs-wrapper/osn-output.hpp"
	"source/obs-wrapper/osn-scene.cpp"
	"source/obs-wrapper/osn-scene.hpp"
	"source/obs-wrapper/osn-sceneitem.cpp"
	"source/obs-wrapper/osn-sceneitem.hpp"
	"source/obs-wrapper/osn-service.cpp"
	"source/obs-wrapper/osn-service.hpp"
	"source/obs-wrapper/osn-source.cpp"
	"source/obs-wrapper/osn-source.hpp"
	"source/obs-wrapper/osn-transition.cpp"
	"source/obs-wrapper/osn-transition.hpp"
	"source/obs-wrapper/osn-video.cpp"
	"source/obs-wrapper/osn-video.hpp"
	"source/obs-wrapper/osn-volmeter.cpp"
	"source/obs-wrapper/osn-volmeter.hpp"

	###### utlity graphics ######
	"source/obs-wrapper/gs-limits.h"
	"source/obs-wrapper/gs-vertex.h"
	"source/obs-wrapper/gs-vertex.cpp"
	"source/obs-wrapper/gs-vertexbuffer.h"
	"source/obs-wrapper/gs-vertexbuffer.cpp"

	###### node-obs ######
	"source/obs-wrapper/obs-nodeobs_api.cpp"
	"source/obs-wrapper/obs-nodeobs_api.h"
	"source/obs-wrapper/nodeobs_audio_encoders.cpp"
	"source/obs-wrapper/nodeobs_audio_encoders.h"
	"source/obs-wrapper/obs-nodeobs_autoconfig.cpp"
	"source/obs-wrapper/obs-nodeobs_autoconfig.h"
	"source/obs-wrapper/nodeobs_configManager.cpp"
	"source/obs-wrapper/nodeobs_configManager.hpp"
	"source/obs-wrapper/obs-nodeobs_display.cpp"
	"source/obs-wrapper/obs-nodeobs_display.h"
	"source/obs-wrapper/nodeobs_content.h"
	"source/obs-wrapper/nodeobs_common.cpp"
	"source/obs-wrapper/obs-nodeobs_service.cpp"
	"source/obs-wrapper/obs-nodeobs_service.h"
	"source/obs-wrapper/util-memory.cpp"
	"source/obs-wrapper/util-memory.h"

	###### crash-manager ######
	"source/obs-wrapper/util-crashmanager.cpp"
	"source/obs-wrapper/util-crashmanager.h"
	"source/obs-wrapper/util-metricsprovider.cpp"
	"source/obs-wrapper/util-metricsprovider.h"

	###### memory-manager ######
	"source/obs-wrapper/memory-manager.cpp"
	"source/obs-wrapper/memory-manager.h"
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

if(WIN32)
	target_sources(obs_studio_client PUBLIC ${PROJECT_BINARY_DIR}/version.rc)
	target_link_libraries(obs_studio_client ${LIBOBS_LIBRARIES} crashpad_client crashpad_util crashpad_base dwmapi.lib)
else ()
	target_link_libraries(obs_studio_client ${LIBOBS_LIBRARIES} ${COREFOUNDATION} ${COCOA} ${AVFOUNDATION} ${Security} ${Foundation} crashpad)
endif()

target_include_directories(
	obs_studio_client
	PUBLIC
		"${CMAKE_SOURCE_DIR}/source/"
		${LIBOBS_INCLUDE_DIRS}
		"${stackwalker_SOURCE_DIR}/Main/StackWalker"
		"${nlohmannjson_SOURCE_DIR}/single_include"
		"${source_dir}/include/third_party/mini_chromium/mini_chromium"
		"${source_dir}/include"
		"${thread-pool_SOURCE_DIR}"
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
add_definitions(-DNAPI_VERSION=7)

if(MSVC)
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /std:c++17")
else()
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++17")
	add_definitions(-std=c++17)
endif()

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
