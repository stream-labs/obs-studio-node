cmake_minimum_required(VERSION 3.1)
project(obs-studio-node VERSION 1.0.0)

#############################
# CMake Settings
#############################

set(PROJECT_NAME "obs_studio_node")
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake")

include(ExternalProject)
include(DownloadProject)
include(CPack)
include(NodeJS)

set(STREAMLABS_BUILD false CACHE BOOL "Whether to build for streamlabs-obs")

if (STREAMLABS_BUILD)
	nodejs_init(
		URL https://atom.io/download/atom-shell
		NAME iojs
		VERSION v1.8.4
	)
else()
	nodejs_init()
endif()

###########################################
# Add libobs as an imported project

# Since building libobs takes a bit of time
# and the build should be reproducible anyways,
# we build obs separately and download
# prebuilt binaries of libobs.

# The format of the downloaded blob follows
# what libobs installs into or what cpack
# provides in a zip file (which should be
# the same).
###########################################

download_project(
	PROJ libobs
	URL https://ci.appveyor.com/api/buildjobs/msdjo84kk48292ga/artifacts/windows-x86_64-dev.zip
)

include("${libobs_SOURCE_DIR}/cmake/LibObs/LibObsTarget.cmake")

#############################
# Source, Libraries & Directories
#############################
set(PROJECT_SOURCE 
	"src/obs-node.cpp"
	"src/Service.cpp" "src/Service.h"
	"src/AudioControls.cpp" "src/AudioControls.h"
	"src/Calldata.cpp" "src/Calldata.h"
	"src/Common.cpp" "src/Common.h"
	"src/Properties.cpp" "src/Properties.h"
	"src/Transition.cpp" "src/Transition.h"
	"src/Filter.cpp" "src/Filter.h"
	"src/SceneItem.cpp" "src/SceneItem.h"
	"src/Scene.cpp" "src/Scene.h"
	"src/IEncoder.cpp" "src/IEncoder.h"
	"src/Display.cpp" "src/Display.h"
	"src/Global.cpp" "src/Global.h"
	"src/Input.cpp" "src/Input.h"
	"src/Output.cpp" "src/Output.h"
	"src/ISource.cpp" "src/ISource.h"
	"src/Module.cpp" "src/Module.h"
	"src/Audio.cpp" "src/Audio.h"
	"src/Video.cpp" "src/Video.h"
	"src/obspp/obspp.cpp" "src/obspp/obspp.hpp"
	"src/obspp/obspp-audio.cpp" "src/obspp/obspp-audio.hpp"
	"src/obspp/obspp-display.cpp" "src/obspp/obspp-display.hpp"
	"src/obspp/obspp-encoder.cpp" "src/obspp/obspp-encoder.hpp"
	"src/obspp/obspp-filter.cpp" "src/obspp/obspp-filter.hpp"
	"src/obspp/obspp-input.cpp" "src/obspp/obspp-input.hpp"
	"src/obspp/obspp-output.cpp" "src/obspp/obspp-output.hpp"
	"src/obspp/obspp-module.cpp" "src/obspp/obspp-module.hpp"
	"src/obspp/obspp-output.hpp"
	"src/obspp/obspp-properties.cpp" "src/obspp/obspp-properties.hpp"
	"src/obspp/obspp-scene.cpp" "src/obspp/obspp-scene.hpp"
	"src/obspp/obspp-service.cpp" "src/obspp/obspp-service.hpp"
	"src/obspp/obspp-source.cpp" "src/obspp/obspp-source.hpp"
	"src/obspp/obspp-transition.cpp" "src/obspp/obspp-transition.hpp"
	"src/obspp/obspp-video.cpp" "src/obspp/obspp-video.hpp"
	"src/obspp/obspp-view.cpp" "src/obspp/obspp-view.hpp"
	"src/nodeobs_module.h" "src/nodeobs_module.cpp"
	"src/nodeobs_api.h" "src/nodeobs_api.cpp"
	"src/nodeobs_audio_encoders.h" "src/nodeobs_audio_encoders.cpp"
	"src/nodeobs_autoconfig.h" "src/nodeobs_autoconfig.cpp"
	"src/nodeobs_common.cpp"
	"src/nodeobs_display.h" "src/nodeobs_display.cpp"
	"src/nodeobs_service.h" "src/nodeobs_service.cpp"
	"src/nodeobs_settings.h" "src/nodeobs_settings.cpp"
	"src/nodeobs_content.h"
	"src/gs-limits.h"
	"src/gs-vertex.h" "src/gs-vertex.cpp"
	"src/gs-vertexbuffer.h" "src/gs-vertexbuffer.cpp"
	"src/util-memory.h" "src/util-memory.cpp"
)

set(PROJECT_EXTRAS
	"package.json"
	"js/index.d.ts"	"js/index.js"
	"js/module.d.ts" "js/module.js"
	"js/type_check.d.ts" "js/type_check.js")

set(PROJECT_RESOURCES resources)

#############################
# Building
#############################
add_nodejs_module(${PROJECT_NAME} ${PROJECT_SOURCE})

# Delay loading of obs.dll so we can decide
# where to load obs.dll from during runtime.
# We must be careful not to call obs functions
# before we load the DLL though.

if(WIN32)
	set_target_properties(${PROJECT_NAME} PROPERTIES
		LINK_FLAGS "/DELAYLOAD:obs.dll")
		
	list(APPEND PROJECT_LIBRARIES "Delayimp.lib")
endif()

target_link_libraries(${PROJECT_NAME} libobs 
	${PROJECT_LIBRARIES}
	${CMAKE_JS_LIB})

target_include_directories(${PROJECT_NAME} PUBLIC
	${CMAKE_JS_INC})


#############################
# Distribute
#############################
install(
	FILES
		$<TARGET_FILE:${PROJECT_NAME}>
		${PROJECT_EXTRAS}
	DESTINATION ${CMAKE_INSTALL_PREFIX})

install(FILES $<TARGET_PDB_FILE:${PROJECT_NAME}> DESTINATION ${CMAKE_INSTALL_PREFIX} OPTIONAL)
install(DIRECTORY ${libobs_SOURCE_DIR}/bin DESTINATION "libobs" PATTERN "*.lib" EXCLUDE)
install(DIRECTORY ${libobs_SOURCE_DIR}/data DESTINATION "libobs")
install(DIRECTORY ${libobs_SOURCE_DIR}/obs-plugins DESTINATION "libobs")
install(DIRECTORY ${PROJECT_RESOURCES}/ DESTINATION "resources")