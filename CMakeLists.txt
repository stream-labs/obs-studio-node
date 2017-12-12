CMAKE_MINIMUM_REQUIRED(VERSION 3.1)
PROJECT(obs-studio-node VERSION 1.0.0)

#############################
# CMake Settings
#############################

# Only allow Debug or Release builds.
SET(CMAKE_CONFIGURATION_TYPES           "Debug;Release")
SET(CMAKE_CXX_FLAGS_RELEASE             "${CMAKE_CXX_FLAGS_RELWITHDEBINFO}")
SET(CMAKE_C_FLAGS_RELEASE               "${CMAKE_C_FLAGS_RELWITHDEBINFO}")
SET(CMAKE_EXE_LINKER_FLAGS_RELEASE      "${CMAKE_EXE_LINKER_FLAGS_RELWITHDEBINFO}")
SET(CMAKE_MODULE_LINKER_FLAGS_RELEASE   "${CMAKE_MODULE_LINKER_FLAGS_RELWITHDEBINFO}")
SET(CMAKE_RC_LINKER_FLAGS_RELEASE       "${CMAKE_RC_LINKER_FLAGS_RELWITHDEBINFO}")
SET(CMAKE_SHARED_LINKER_FLAGS_RELEASE   "${CMAKE_SHARED_LINKER_FLAGS_RELWITHDEBINFO}")
SET(CMAKE_STATIC_LINKER_FLAGS_RELEASE   "${CMAKE_STATIC_LINKER_FLAGS_RELWITHDEBINFO}")

#############################
# Configuration
#############################

# Binary
SET(BINARY_PREFIX "" CACHE STRING "Binary File prefix")
SET(BINARY_NAME "obs-studio-node" CACHE STRING "Binary File name")
SET(BINARY_SUFFIX ".node" CACHE STRING "Binary File suffix")

# Distribution
#SET(ENABLE_INSTALL true CACHE BOOL "Binary File prefix")
SET(INSTALL_DIR "${CMAKE_SOURCE_DIR}/distribute" CACHE PATH "Path at which to install this module")

# OBS Studio
SET(OBSSTUDIO_PREBUILT true CACHE BOOL "Use prebuilt binaries?")
SET(OBSSTUDIO_PATH "${CMAKE_SOURCE_DIR}/obs-prebuild" CACHE PATH "Path to OBS Studio or prebuilt binaries")

#############################
# OBS Specific Stuff
#############################
if(OBSSTUDIO_PREBUILT)
	SET(OBSSTUDIO_INCLUDE
		"${OBSSTUDIO_PATH}/include"
	)
	SET(OBSSTUDIO_LIBRARIES
		debug "${OBSSTUDIO_PATH}/bin/Debug/obs.lib"
		debug "${OBSSTUDIO_PATH}/bin/Debug/w32-pthreads.lib"
		optimized "${OBSSTUDIO_PATH}/bin/Release/obs.lib"
		optimized "${OBSSTUDIO_PATH}/bin/Release/w32-pthreads.lib"
	)
	SET(OBSSTUDIO_PLUGINS_DATA
		"${OBSSTUDIO_PATH}/data/"
	)
	SET(OBSSTUDIO_BINARIES_DEBUG
		"${OBSSTUDIO_PATH}/bin/Debug/obs.dll"
		"${OBSSTUDIO_PATH}/bin/Debug/obs.pdb"
		"${OBSSTUDIO_PATH}/bin/Debug/obsglad.dll"
		"${OBSSTUDIO_PATH}/bin/Debug/obsglad.pdb"
		"${OBSSTUDIO_PATH}/bin/Debug/w32-pthreads.dll"
		"${OBSSTUDIO_PATH}/bin/Debug/w32-pthreads.pdb"
		"${OBSSTUDIO_PATH}/bin/Debug/avcodec-57.dll"
		"${OBSSTUDIO_PATH}/bin/Debug/avdevice-57.dll"
		"${OBSSTUDIO_PATH}/bin/Debug/avfilter-6.dll"
		"${OBSSTUDIO_PATH}/bin/Debug/avformat-57.dll"
		"${OBSSTUDIO_PATH}/bin/Debug/avutil-55.dll"
		"${OBSSTUDIO_PATH}/bin/Debug/libcurl.dll"
		"${OBSSTUDIO_PATH}/bin/Debug/libEGLd.dll"
		"${OBSSTUDIO_PATH}/bin/Debug/libGLESv2d.dll"
		"${OBSSTUDIO_PATH}/bin/Debug/libobs-d3d11.dll"
		"${OBSSTUDIO_PATH}/bin/Debug/libobs-opengl.dll"
		"${OBSSTUDIO_PATH}/bin/Debug/libogg-0.dll"
		"${OBSSTUDIO_PATH}/bin/Debug/libopus-0.dll"
		"${OBSSTUDIO_PATH}/bin/Debug/libvorbis-0.dll"
		"${OBSSTUDIO_PATH}/bin/Debug/libvorbisenc-2.dll"
		"${OBSSTUDIO_PATH}/bin/Debug/libvorbisfile-3.dll"
		"${OBSSTUDIO_PATH}/bin/Debug/libvpx-1.dll"
		"${OBSSTUDIO_PATH}/bin/Debug/libx264-148.dll"
		"${OBSSTUDIO_PATH}/bin/Debug/swresample-2.dll"
		"${OBSSTUDIO_PATH}/bin/Debug/swscale-4.dll"
		"${OBSSTUDIO_PATH}/bin/Debug/zlib.dll"
	)
	SET(OBSSTUDIO_PLUGINS_DEBUG "${OBSSTUDIO_PATH}/obs-plugins/Debug/")
	SET(OBSSTUDIO_BINARIES_RELEASE
		"${OBSSTUDIO_PATH}/bin/Release/obs.dll"
		"${OBSSTUDIO_PATH}/bin/Release/obs.pdb"
		"${OBSSTUDIO_PATH}/bin/Release/obsglad.dll"
		"${OBSSTUDIO_PATH}/bin/Release/obsglad.pdb"
		"${OBSSTUDIO_PATH}/bin/Release/w32-pthreads.dll"
		"${OBSSTUDIO_PATH}/bin/Release/w32-pthreads.pdb"
		"${OBSSTUDIO_PATH}/bin/Release/avcodec-57.dll"
		"${OBSSTUDIO_PATH}/bin/Release/avdevice-57.dll"
		"${OBSSTUDIO_PATH}/bin/Release/avfilter-6.dll"
		"${OBSSTUDIO_PATH}/bin/Release/avformat-57.dll"
		"${OBSSTUDIO_PATH}/bin/Release/avutil-55.dll"
		"${OBSSTUDIO_PATH}/bin/Release/libcurl.dll"
		"${OBSSTUDIO_PATH}/bin/Release/libEGL.dll"
		"${OBSSTUDIO_PATH}/bin/Release/libGLESv2.dll"
		"${OBSSTUDIO_PATH}/bin/Release/libobs-d3d11.dll"
		"${OBSSTUDIO_PATH}/bin/Release/libobs-opengl.dll"
		"${OBSSTUDIO_PATH}/bin/Release/libogg-0.dll"
		"${OBSSTUDIO_PATH}/bin/Release/libopus-0.dll"
		"${OBSSTUDIO_PATH}/bin/Release/libvorbis-0.dll"
		"${OBSSTUDIO_PATH}/bin/Release/libvorbisenc-2.dll"
		"${OBSSTUDIO_PATH}/bin/Release/libvorbisfile-3.dll"
		"${OBSSTUDIO_PATH}/bin/Release/libvpx-1.dll"
		"${OBSSTUDIO_PATH}/bin/Release/libx264-148.dll"
		"${OBSSTUDIO_PATH}/bin/Release/swresample-2.dll"
		"${OBSSTUDIO_PATH}/bin/Release/swscale-4.dll"
		"${OBSSTUDIO_PATH}/bin/Release/zlib.dll"
	)
	SET(OBSSTUDIO_PLUGINS_RELEASE "${OBSSTUDIO_PATH}/obs-plugins/Release/")
ENDIF()	

#############################
# Source, Libraries & Directories
#############################
SET(PROJECT_SOURCE 
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
	"src/nodeobs_audio.h" "src/nodeobs_audio.cpp"
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
)
SET(PROJECT_LIBRARIES
	${CMAKE_JS_LIB}
	${OBSSTUDIO_LIBRARIES}
)
SET(PROJECT_INCLUDE_PATHS
	${CMAKE_JS_INC}
	${OBSSTUDIO_INCLUDE}
)
SET(PROJECT_DATA
	"resources/"
)
ADD_DEFINITIONS(-DBUILDING_NODE_EXTENSION)

#############################
# Building
#############################
ADD_LIBRARY(obs-studio-node SHARED ${PROJECT_SOURCE})
TARGET_LINK_LIBRARIES(obs-studio-node ${PROJECT_LIBRARIES})
TARGET_INCLUDE_DIRECTORIES(obs-studio-node PUBLIC ${PROJECT_INCLUDE_PATHS})
SET_TARGET_PROPERTIES(obs-studio-node PROPERTIES PREFIX "${BINARY_PREFIX}" SUFFIX "${BINARY_SUFFIX}")

#############################
# Distribute
#############################
INSTALL(TARGETS obs-studio-node DESTINATION "${INSTALL_DIR}")
INSTALL(FILES $<TARGET_PDB_FILE:obs-studio-node> DESTINATION "${INSTALL_DIR}" OPTIONAL)
INSTALL(DIRECTORY ${PROJECT_DATA} DESTINATION "${INSTALL_DIR}/resources/" OPTIONAL)
INSTALL(FILES ${OBSSTUDIO_BINARIES_DEBUG} CONFIGURATIONS Debug DESTINATION "${INSTALL_DIR}")
INSTALL(DIRECTORY ${OBSSTUDIO_PLUGINS_DEBUG} CONFIGURATIONS Debug DESTINATION "${INSTALL_DIR}/obs-plugins/")
INSTALL(FILES ${OBSSTUDIO_BINARIES_RELEASE} CONFIGURATIONS Release DESTINATION "${INSTALL_DIR}")
INSTALL(DIRECTORY ${OBSSTUDIO_PLUGINS_RELEASE} CONFIGURATIONS Release DESTINATION "${INSTALL_DIR}/obs-plugins/")
INSTALL(DIRECTORY ${OBSSTUDIO_PLUGINS_DATA} DESTINATION "${INSTALL_DIR}/data/")
