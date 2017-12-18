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
