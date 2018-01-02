CMAKE_MINIMUM_REQUIRED(VERSION 3.0)
PROJECT(obs-studio-server VERSION 0.0.1)

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

# OBS Studio
SET(OBSSTUDIO_PREBUILT true CACHE BOOL "Use prebuilt binaries?")
SET(OBSSTUDIO_PATH "${PROJECT_SOURCE_DIR}/obs-prebuild" CACHE PATH "Path to OBS Studio or prebuilt binaries")

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
	"${PROJECT_SOURCE_DIR}/source/main.cpp"
	"${PROJECT_SOURCE_DIR}/source/shared.cpp" "${PROJECT_SOURCE_DIR}/source/shared.hpp"
	"${PROJECT_SOURCE_DIR}/source/utility.cpp" "${PROJECT_SOURCE_DIR}/source/utility.hpp"
	"${PROJECT_SOURCE_DIR}/source/obs-main.hpp" "${PROJECT_SOURCE_DIR}/source/obs-main.cpp"
	
	"${PROJECT_SOURCE_DIR}/source/old/obs-node.cpp"
	"${PROJECT_SOURCE_DIR}/source/old/Service.cpp" "${PROJECT_SOURCE_DIR}/source/old/Service.h"
	"${PROJECT_SOURCE_DIR}/source/old/AudioControls.cpp" "${PROJECT_SOURCE_DIR}/source/old/AudioControls.h"
	"${PROJECT_SOURCE_DIR}/source/old/Calldata.cpp" "${PROJECT_SOURCE_DIR}/source/old/Calldata.h"
	"${PROJECT_SOURCE_DIR}/source/old/Common.cpp" "${PROJECT_SOURCE_DIR}/source/old/Common.h"
	"${PROJECT_SOURCE_DIR}/source/old/Properties.cpp" "${PROJECT_SOURCE_DIR}/source/old/Properties.h"
	"${PROJECT_SOURCE_DIR}/source/old/Transition.cpp" "${PROJECT_SOURCE_DIR}/source/old/Transition.h"
	"${PROJECT_SOURCE_DIR}/source/old/Filter.cpp" "${PROJECT_SOURCE_DIR}/source/old/Filter.h"
	"${PROJECT_SOURCE_DIR}/source/old/SceneItem.cpp" "${PROJECT_SOURCE_DIR}/source/old/SceneItem.h"
	"${PROJECT_SOURCE_DIR}/source/old/Scene.cpp" "${PROJECT_SOURCE_DIR}/source/old/Scene.h"
	"${PROJECT_SOURCE_DIR}/source/old/IEncoder.cpp" "${PROJECT_SOURCE_DIR}/source/old/IEncoder.h"
	"${PROJECT_SOURCE_DIR}/source/old/Display.cpp" "${PROJECT_SOURCE_DIR}/source/old/Display.h"
	"${PROJECT_SOURCE_DIR}/source/old/Global.cpp" "${PROJECT_SOURCE_DIR}/source/old/Global.h"
	"${PROJECT_SOURCE_DIR}/source/old/Input.cpp" "${PROJECT_SOURCE_DIR}/source/old/Input.h"
	"${PROJECT_SOURCE_DIR}/source/old/Output.cpp" "${PROJECT_SOURCE_DIR}/source/old/Output.h"
	"${PROJECT_SOURCE_DIR}/source/old/ISource.cpp" "${PROJECT_SOURCE_DIR}/source/old/ISource.h"
	"${PROJECT_SOURCE_DIR}/source/old/Module.cpp" "${PROJECT_SOURCE_DIR}/source/old/Module.h"
	"${PROJECT_SOURCE_DIR}/source/old/Audio.cpp" "${PROJECT_SOURCE_DIR}/source/old/Audio.h"
	"${PROJECT_SOURCE_DIR}/source/old/Video.cpp" "${PROJECT_SOURCE_DIR}/source/old/Video.h"
	"${PROJECT_SOURCE_DIR}/source/old/obspp/obspp.cpp" "${PROJECT_SOURCE_DIR}/source/old/obspp/obspp.hpp"
	"${PROJECT_SOURCE_DIR}/source/old/obspp/obspp-audio.cpp" "${PROJECT_SOURCE_DIR}/source/old/obspp/obspp-audio.hpp"
	"${PROJECT_SOURCE_DIR}/source/old/obspp/obspp-display.cpp" "${PROJECT_SOURCE_DIR}/source/old/obspp/obspp-display.hpp"
	"${PROJECT_SOURCE_DIR}/source/old/obspp/obspp-encoder.cpp" "${PROJECT_SOURCE_DIR}/source/old/obspp/obspp-encoder.hpp"
	"${PROJECT_SOURCE_DIR}/source/old/obspp/obspp-filter.cpp" "${PROJECT_SOURCE_DIR}/source/old/obspp/obspp-filter.hpp"
	"${PROJECT_SOURCE_DIR}/source/old/obspp/obspp-index.hpp"
	"${PROJECT_SOURCE_DIR}/source/old/obspp/obspp-input.cpp" "${PROJECT_SOURCE_DIR}/source/old/obspp/obspp-input.hpp"
	"${PROJECT_SOURCE_DIR}/source/old/obspp/obspp-output.cpp" "${PROJECT_SOURCE_DIR}/source/old/obspp/obspp-output.hpp"
	"${PROJECT_SOURCE_DIR}/source/old/obspp/obspp-module.cpp" "${PROJECT_SOURCE_DIR}/source/old/obspp/obspp-module.hpp"
	"${PROJECT_SOURCE_DIR}/source/old/obspp/obspp-output.hpp"
	"${PROJECT_SOURCE_DIR}/source/old/obspp/obspp-properties.cpp" "${PROJECT_SOURCE_DIR}/source/old/obspp/obspp-properties.hpp"
	"${PROJECT_SOURCE_DIR}/source/old/obspp/obspp-scene.cpp" "${PROJECT_SOURCE_DIR}/source/old/obspp/obspp-scene.hpp"
	"${PROJECT_SOURCE_DIR}/source/old/obspp/obspp-service.cpp" "${PROJECT_SOURCE_DIR}/source/old/obspp/obspp-service.hpp"
	"${PROJECT_SOURCE_DIR}/source/old/obspp/obspp-source.cpp" "${PROJECT_SOURCE_DIR}/source/old/obspp/obspp-source.hpp"
	"${PROJECT_SOURCE_DIR}/source/old/obspp/obspp-transition.cpp" "${PROJECT_SOURCE_DIR}/source/old/obspp/obspp-transition.hpp"
	"${PROJECT_SOURCE_DIR}/source/old/obspp/obspp-video.cpp" "${PROJECT_SOURCE_DIR}/source/old/obspp/obspp-video.hpp"
	"${PROJECT_SOURCE_DIR}/source/old/obspp/obspp-view.cpp" "${PROJECT_SOURCE_DIR}/source/old/obspp/obspp-view.hpp"
	"${PROJECT_SOURCE_DIR}/source/old/nodeobs_module.h" "${PROJECT_SOURCE_DIR}/source/old/nodeobs_module.cpp"
	"${PROJECT_SOURCE_DIR}/source/old/nodeobs_api.h" "${PROJECT_SOURCE_DIR}/source/old/nodeobs_api.cpp"
	"${PROJECT_SOURCE_DIR}/source/old/nodeobs_audio.h" "${PROJECT_SOURCE_DIR}/source/old/nodeobs_audio.cpp"
	"${PROJECT_SOURCE_DIR}/source/old/nodeobs_audio_encoders.h" "${PROJECT_SOURCE_DIR}/source/old/nodeobs_audio_encoders.cpp"
	"${PROJECT_SOURCE_DIR}/source/old/nodeobs_autoconfig.h" "${PROJECT_SOURCE_DIR}/source/old/nodeobs_autoconfig.cpp"
	"${PROJECT_SOURCE_DIR}/source/old/nodeobs_common.h" "${PROJECT_SOURCE_DIR}/source/old/nodeobs_common.cpp"
	"${PROJECT_SOURCE_DIR}/source/old/nodeobs_display.h" "${PROJECT_SOURCE_DIR}/source/old/nodeobs_display.cpp"
	"${PROJECT_SOURCE_DIR}/source/old/nodeobs_event.h" "${PROJECT_SOURCE_DIR}/source/old/nodeobs_event.cpp"
	"${PROJECT_SOURCE_DIR}/source/old/nodeobs_service.h" "${PROJECT_SOURCE_DIR}/source/old/nodeobs_service.cpp"
	"${PROJECT_SOURCE_DIR}/source/old/nodeobs_settings.h" "${PROJECT_SOURCE_DIR}/source/old/nodeobs_settings.cpp"
	"${PROJECT_SOURCE_DIR}/source/old/nodeobs_obspp_manager.hpp" "${PROJECT_SOURCE_DIR}/source/old/nodeobs_obspp_manager.cpp"
	"${PROJECT_SOURCE_DIR}/source/old/nodeobs_obspp_index.hpp" "${PROJECT_SOURCE_DIR}/source/old/nodeobs_obspp_index.cpp"
	"${PROJECT_SOURCE_DIR}/source/old/nodeobs_content.h"
)
SET(PROJECT_LIBRARIES
	${OBSSTUDIO_LIBRARIES}
	lib-streamlabs-ipc
)
SET(PROJECT_INCLUDE_PATHS
	${OBSSTUDIO_INCLUDE}
	"${PROJECT_SOURCE_DIR}/source"
	"${CMAKE_SOURCE_DIR}/lib-streamlabs-ipc/include"
)

#############################
# Building
#############################
ADD_EXECUTABLE(obs-studio-server ${PROJECT_SOURCE})
TARGET_LINK_LIBRARIES(obs-studio-server ${PROJECT_LIBRARIES})
TARGET_INCLUDE_DIRECTORIES(obs-studio-server PUBLIC ${PROJECT_INCLUDE_PATHS})

#############################
# Distribute
#############################
INSTALL(TARGETS obs-studio-server DESTINATION "${INSTALL_DIR}")
INSTALL(FILES $<TARGET_PDB_FILE:obs-studio-server> DESTINATION "${INSTALL_DIR}" OPTIONAL)
INSTALL(DIRECTORY ${PROJECT_DATA} DESTINATION "${INSTALL_DIR}/resources/" OPTIONAL)
INSTALL(FILES ${OBSSTUDIO_BINARIES_DEBUG} CONFIGURATIONS Debug DESTINATION "${INSTALL_DIR}")
INSTALL(DIRECTORY ${OBSSTUDIO_PLUGINS_DEBUG} CONFIGURATIONS Debug DESTINATION "${INSTALL_DIR}/obs-plugins/")
INSTALL(FILES ${OBSSTUDIO_BINARIES_RELEASE} CONFIGURATIONS Release DESTINATION "${INSTALL_DIR}")
INSTALL(DIRECTORY ${OBSSTUDIO_PLUGINS_RELEASE} CONFIGURATIONS Release DESTINATION "${INSTALL_DIR}/obs-plugins/")
INSTALL(DIRECTORY ${OBSSTUDIO_PLUGINS_DATA} DESTINATION "${INSTALL_DIR}/data/")
