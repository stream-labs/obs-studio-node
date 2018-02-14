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
		"${OBSSTUDIO_PATH}/bin/Debug/obs.dll" "${OBSSTUDIO_PATH}/bin/Debug/obs.pdb"
		"${OBSSTUDIO_PATH}/bin/Debug/obsglad.dll" "${OBSSTUDIO_PATH}/bin/Debug/obsglad.pdb"
		"${OBSSTUDIO_PATH}/bin/Debug/w32-pthreads.dll" "${OBSSTUDIO_PATH}/bin/Debug/w32-pthreads.pdb"
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
		"${OBSSTUDIO_PATH}/bin/Release/obs.dll" "${OBSSTUDIO_PATH}/bin/Release/obs.pdb"
		"${OBSSTUDIO_PATH}/bin/Release/obsglad.dll" "${OBSSTUDIO_PATH}/bin/Release/obsglad.pdb"
		"${OBSSTUDIO_PATH}/bin/Release/w32-pthreads.dll" "${OBSSTUDIO_PATH}/bin/Release/w32-pthreads.pdb"
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
	"${PROJECT_SOURCE_DIR}/source/osn-nodeobs.cpp" "${PROJECT_SOURCE_DIR}/source/osn-nodeobs.hpp"
	"${PROJECT_SOURCE_DIR}/source/osn-audio.cpp" "${PROJECT_SOURCE_DIR}/source/osn-audio.hpp"
	"${PROJECT_SOURCE_DIR}/source/osn-audiocontrols.cpp" "${PROJECT_SOURCE_DIR}/source/osn-audiocontrols.hpp"
	"${PROJECT_SOURCE_DIR}/source/osn-calldata.cpp" "${PROJECT_SOURCE_DIR}/source/osn-calldata.hpp"
	"${PROJECT_SOURCE_DIR}/source/osn-common.cpp" "${PROJECT_SOURCE_DIR}/source/osn-common.hpp"
	"${PROJECT_SOURCE_DIR}/source/osn-display.cpp" "${PROJECT_SOURCE_DIR}/source/osn-display.hpp"
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
