cmake_minimum_required(VERSION 3.1)
project(obs-studio-node VERSION 1.0.0)

#############################
# CMake Settings
#############################

set(PROJECT_NAME "obs_studio_node")
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake")

include(ExternalProject)
include(DownloadProject)
include(NodeJS)

nodejs_init()

# Only allow Debug or Release builds.
set(CMAKE_CONFIGURATION_TYPES           "Debug;Release")
set(CMAKE_CXX_FLAGS_RELEASE             "${CMAKE_CXX_FLAGS_RELWITHDEBINFO}")
set(CMAKE_C_FLAGS_RELEASE               "${CMAKE_C_FLAGS_RELWITHDEBINFO}")
set(CMAKE_EXE_LINKER_FLAGS_RELEASE      "${CMAKE_EXE_LINKER_FLAGS_RELWITHDEBINFO}")
set(CMAKE_MODULE_LINKER_FLAGS_RELEASE   "${CMAKE_MODULE_LINKER_FLAGS_RELWITHDEBINFO}")
set(CMAKE_RC_LINKER_FLAGS_RELEASE       "${CMAKE_RC_LINKER_FLAGS_RELWITHDEBINFO}")
set(CMAKE_SHARED_LINKER_FLAGS_RELEASE   "${CMAKE_SHARED_LINKER_FLAGS_RELWITHDEBINFO}")
set(CMAKE_STATIC_LINKER_FLAGS_RELEASE   "${CMAKE_STATIC_LINKER_FLAGS_RELWITHDEBINFO}")

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

# Now, cmake is a bit dumb. You cannot 
# normally have it download files and
# unpack them during configure time. The
# problem with this is if we want a flexible
# build system, and we want to use imported
# targets... we have to. So we instead make
# a separate project that downloads the
# binary during build time then build it
# during configure time. Why doesn't cmake
# have something to do this for us or why
# doesn't it allow to add an external project
# during configure time? We'll probably never know.
###########################################

download_project(
	PROJ libobs
	URL https://ci.appveyor.com/api/buildjobs/io1e23q5s8nt0n1x/artifacts/windows-x86_64-dev.zip
	UPDATE_DISCONNECTED 1
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
	"src/obspp/obspp-index.hpp"
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
	"src/nodeobs_common.h" "src/nodeobs_common.cpp"
	"src/nodeobs_display.h" "src/nodeobs_display.cpp"
	"src/nodeobs_event.h" "src/nodeobs_event.cpp"
	"src/nodeobs_service.h" "src/nodeobs_service.cpp"
	"src/nodeobs_settings.h" "src/nodeobs_settings.cpp"
	"src/nodeobs_obspp_manager.hpp" "src/nodeobs_obspp_manager.cpp"
	"src/nodeobs_obspp_index.hpp" "src/nodeobs_obspp_index.cpp"
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
		LINK_FLAGS_RELEASE "/DELAYLOAD:obs.dll"
		LINK_FLAGS_DEBUG   "/DELAYLOAD:obs.dll")
		
	list(APPEND PROJECT_LIBRARIES "Delayimp.lib")

endif()
target_link_libraries(${PROJECT_NAME} libobs ${PROJECT_LIBRARIES} ${CMAKE_JS_LIB} ${PROJECT_LINKER_FLAGS})

target_include_directories(${PROJECT_NAME} PUBLIC
	$<TARGET_PROPERTY:libobs,INTERFACE_INCLUDE_DIRECTORIES>
	${CMAKE_JS_INC})


#############################
# Distribute
#############################
install(TARGETS ${PROJECT_NAME} DESTINATION ${CMAKE_INSTALL_PREFIX})
install(FILES $<TARGET_PDB_FILE:${PROJECT_NAME}> DESTINATION ${CMAKE_INSTALL_PREFIX})
install(FILES ${PROJECT_EXTRAS} DESTINATION ${CMAKE_INSTALL_PREFIX})
install(DIRECTORY ${libobs_SOURCE_DIR}/bin DESTINATION "libobs" PATTERN "*.lib" EXCLUDE)
install(DIRECTORY ${libobs_SOURCE_DIR}/data DESTINATION "libobs")
install(DIRECTORY ${libobs_SOURCE_DIR}/obs-plugins DESTINATION "libobs")
install(DIRECTORY ${PROJECT_RESOURCES}/ DESTINATION "resources")