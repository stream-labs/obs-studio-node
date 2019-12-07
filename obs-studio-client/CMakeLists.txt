PROJECT(obs_studio_client VERSION ${obs-studio-node_VERSION})

SET(NODEJS_URL "https://atom.io/download/atom-shell" CACHE STRING "Node.JS URL")
SET(NODEJS_NAME "iojs" CACHE STRING "Node.JS Name")
SET(NODEJS_VERSION "v6.0.3" CACHE STRING "Node.JS Version")

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

############################
# Setup crashpad binaries
############################
download_project(
	PROJ crashpad
	URL https://obsstudionodes3.streamlabs.com/crashpad/crashpad-release-1.0.21-win-x64.tar.gz
	UPDATE_DISCONNECTED 1
)

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

string(
	CONCAT OSN_DEFAULT_LIBOBS_URL
	"https://obsstudios3.streamlabs.com/"
	"libobs-${LIBOBS_BUILD_TYPE}-${LIBOBS_VERSION}.7z"
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

add_nodejs_module(
	obs_studio_client
	"${CMAKE_SOURCE_DIR}/source/error.hpp"
	"${CMAKE_SOURCE_DIR}/source/obs-property.hpp"
	"${CMAKE_SOURCE_DIR}/source/obs-property.cpp"

	"source/shared.cpp"
	"source/shared.hpp"
	"source/utility.cpp"
	"source/utility.hpp"
	"source/utility-v8.cpp"
	"source/utility-v8.hpp"
	"source/controller.cpp"
	"source/controller.hpp"
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

	#### Server ####
	"../obs-studio-server/source/osn-nodeobs.cpp"

		###### obs-studio-node ######
	"../obs-studio-server/source/shared.cpp"
	"../obs-studio-server/source/shared.hpp"
	"../obs-studio-server/source/utility.cpp"
	"../obs-studio-server/source/utility.hpp"
	"../obs-studio-server/source/osn-nodeobs.cpp"
	"../obs-studio-server/source/osn-nodeobs.hpp"
	"../obs-studio-server/source/osn-audio.cpp"
	"../obs-studio-server/source/osn-audio.hpp"
	"../obs-studio-server/source/osn-calldata.cpp"
	"../obs-studio-server/source/osn-calldata.hpp"
	"../obs-studio-server/source/osn-common.cpp"
	"../obs-studio-server/source/osn-common.hpp"
	"../obs-studio-server/source/osn-display.cpp"
	"../obs-studio-server/source/osn-display.hpp"
	"../obs-studio-server/source/osn-fader.cpp"
	"../obs-studio-server/source/osn-fader.hpp"
	"../obs-studio-server/source/osn-filter.cpp"
	"../obs-studio-server/source/osn-filter.hpp"
	"../obs-studio-server/source/osn-global.cpp"
	"../obs-studio-server/source/osn-global.hpp"
	"../obs-studio-server/source/osn-iencoder.cpp"
	"../obs-studio-server/source/osn-iencoder.hpp"
	"../obs-studio-server/source/osn-input.cpp"
	"../obs-studio-server/source/osn-input.hpp"
	"../obs-studio-server/source/osn-module.cpp"
	"../obs-studio-server/source/osn-module.hpp"
	"../obs-studio-server/source/osn-output.cpp"
	"../obs-studio-server/source/osn-output.hpp"
	"../obs-studio-server/source/osn-properties.cpp"
	"../obs-studio-server/source/osn-properties.hpp"
	"../obs-studio-server/source/osn-scene.cpp"
	"../obs-studio-server/source/osn-scene.hpp"
	"../obs-studio-server/source/osn-sceneitem.cpp"
	"../obs-studio-server/source/osn-sceneitem.hpp"
	"../obs-studio-server/source/osn-service.cpp"
	"../obs-studio-server/source/osn-service.hpp"
	"../obs-studio-server/source/osn-source.cpp"
	"../obs-studio-server/source/osn-source.hpp"
	"../obs-studio-server/source/osn-transition.cpp"
	"../obs-studio-server/source/osn-transition.hpp"
	"../obs-studio-server/source/osn-video.cpp"
	"../obs-studio-server/source/osn-video.hpp"
	"../obs-studio-server/source/osn-volmeter.cpp"
	"../obs-studio-server/source/osn-volmeter.hpp"

	###### utlity graphics ######
	"../obs-studio-server/source/gs-limits.h"
	"../obs-studio-server/source/gs-vertex.h"
	"../obs-studio-server/source/gs-vertex.cpp"
	"../obs-studio-server/source/gs-vertexbuffer.h"
	"../obs-studio-server/source/gs-vertexbuffer.cpp"

	###### node-obs ######
	"../obs-studio-server/source/server-nodeobs_api.cpp"
	"../obs-studio-server/source/server-nodeobs_api.h"
	"../obs-studio-server/source/server-nodeobs_audio_encoders.cpp"
	"../obs-studio-server/source/server-nodeobs_audio_encoders.h"
	"../obs-studio-server/source/server-nodeobs_autoconfig.cpp"
	"../obs-studio-server/source/server-nodeobs_autoconfig.h"
	"../obs-studio-server/source/server-nodeobs_configManager.cpp"
	"../obs-studio-server/source/server-nodeobs_configManager.hpp"
	"../obs-studio-server/source/server-nodeobs_display.cpp"
	"../obs-studio-server/source/server-nodeobs_display.h"
	"../obs-studio-server/source/server-nodeobs_content.h"
	"../obs-studio-server/source/server-nodeobs_common.cpp"
	"../obs-studio-server/source/server-nodeobs_service.cpp"
	"../obs-studio-server/source/server-nodeobs_service.h"
	"../obs-studio-server/source/server-nodeobs_settings.cpp"
	"../obs-studio-server/source/server-nodeobs_settings.h"
	"../obs-studio-server/source/util-memory.cpp"
	"../obs-studio-server/source/util-memory.h"

	###### crash-manager ######
	"../obs-studio-server/source/util-crashmanager.cpp"
	"../obs-studio-server/source/util-crashmanager.h"
	"../obs-studio-server/source/util-metricsprovider.cpp"
	"../obs-studio-server/source/util-metricsprovider.h"
	
	###### callback-manager ######
	"../obs-studio-server/source/callback-manager.cpp"
	"../obs-studio-server/source/callback-manager.h"

	###### memory-manager ######
	"../obs-studio-server/source/memory-manager.cpp"
	"../obs-studio-server/source/memory-manager.h"
)

set(PROJECT_INCLUDE_PATHS
	"${nlohmannjson_SOURCE_DIR}/single_include"
	${LIBOBS_INCLUDE_DIRS}
	"${stackwalker_SOURCE_DIR}/Main/StackWalker"
)

if(WIN32)
	target_sources(obs_studio_client PUBLIC ${PROJECT_BINARY_DIR}/version.rc)
endif()

target_link_libraries(obs_studio_client lib-streamlabs-ipc ${LIBOBS_LIBRARIES} optimized crashpad StackWalker)

target_include_directories(
	obs_studio_client
	PUBLIC
		"${CMAKE_SOURCE_DIR}/source/"
		"${CMAKE_SOURCE_DIR}/lib-streamlabs-ipc/include"
		${PROJECT_INCLUDE_PATHS}
)

set(PROJECT_DATA "../obs-studio-server/resources")
 
set_target_properties(
	obs_studio_client
	PROPERTIES
	VERSION ${PROJECT_VERSION}
	SOVERSION ${PROJECT_VERSION}
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

IF( NOT CLANG_ANALYZE_CONFIG)
	install(FILES $<TARGET_PDB_FILE:obs_studio_client> DESTINATION ./ OPTIONAL)
ENDIF()
install(FILES "${CMAKE_SOURCE_DIR}/package.json" DESTINATION "./")

install(
	DIRECTORY "${PROJECT_SOURCE_DIR}/../js/"
	DESTINATION "./"
	PATTERN "*.json" EXCLUDE
)
install(DIRECTORY ${PROJECT_DATA} DESTINATION "./" OPTIONAL)
install(DIRECTORY ${crashpad_SOURCE_DIR}/bin/ DESTINATION "./")
install(DIRECTORY "${libobs_SOURCE_DIR}/data/" DESTINATION "./data")

install(
	DIRECTORY "${libobs_SOURCE_DIR}/bin/64bit/"
	DESTINATION "./" PATTERN "*.lib" EXCLUDE
)

install(
	DIRECTORY "${libobs_SOURCE_DIR}/obs-plugins/"
	DESTINATION "./obs-plugins"
)

install(
	TARGETS obs_studio_client
	RUNTIME DESTINATION "./" COMPONENT Runtime
	LIBRARY DESTINATION "./" COMPONENT Runtime
	ARCHIVE DESTINATION "./" COMPONENT Runtime	
)
