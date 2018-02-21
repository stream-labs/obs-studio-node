CMAKE_MINIMUM_REQUIRED(VERSION 3.1)
PROJECT(obs-studio-client VERSION 1.0.0)

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
# Source, Libraries & Directories
#############################
SET(PROJECT_SOURCE 
	"${PROJECT_SOURCE_DIR}/source/main.cpp"
	"${PROJECT_SOURCE_DIR}/source/shared.cpp" "${PROJECT_SOURCE_DIR}/source/shared.hpp"
	"${PROJECT_SOURCE_DIR}/source/utility.cpp" "${PROJECT_SOURCE_DIR}/source/utility.hpp"
	"${PROJECT_SOURCE_DIR}/source/controller.cpp" "${PROJECT_SOURCE_DIR}/source/controller.hpp"
	
	"${PROJECT_SOURCE_DIR}/source/obs-node.cpp"
	"${PROJECT_SOURCE_DIR}/source/Service.cpp" "${PROJECT_SOURCE_DIR}/source/Service.h"
	"${PROJECT_SOURCE_DIR}/source/AudioControls.cpp" "${PROJECT_SOURCE_DIR}/source/AudioControls.h"
	"${PROJECT_SOURCE_DIR}/source/Calldata.cpp" "${PROJECT_SOURCE_DIR}/source/Calldata.h"
	"${PROJECT_SOURCE_DIR}/source/Common.cpp" "${PROJECT_SOURCE_DIR}/source/Common.h"
	"${PROJECT_SOURCE_DIR}/source/Properties.cpp" "${PROJECT_SOURCE_DIR}/source/Properties.h"
	"${PROJECT_SOURCE_DIR}/source/Transition.cpp" "${PROJECT_SOURCE_DIR}/source/Transition.h"
	"${PROJECT_SOURCE_DIR}/source/Filter.cpp" "${PROJECT_SOURCE_DIR}/source/Filter.h"
	"${PROJECT_SOURCE_DIR}/source/SceneItem.cpp" "${PROJECT_SOURCE_DIR}/source/SceneItem.h"
	"${PROJECT_SOURCE_DIR}/source/Scene.cpp" "${PROJECT_SOURCE_DIR}/source/Scene.h"
	"${PROJECT_SOURCE_DIR}/source/IEncoder.cpp" "${PROJECT_SOURCE_DIR}/source/IEncoder.h"
	"${PROJECT_SOURCE_DIR}/source/Display.cpp" "${PROJECT_SOURCE_DIR}/source/Display.h"
	"${PROJECT_SOURCE_DIR}/source/Global.cpp" "${PROJECT_SOURCE_DIR}/source/Global.h"
	"${PROJECT_SOURCE_DIR}/source/Input.cpp" "${PROJECT_SOURCE_DIR}/source/Input.h"
	"${PROJECT_SOURCE_DIR}/source/Output.cpp" "${PROJECT_SOURCE_DIR}/source/Output.h"
	"${PROJECT_SOURCE_DIR}/source/ISource.cpp" "${PROJECT_SOURCE_DIR}/source/ISource.h"
	"${PROJECT_SOURCE_DIR}/source/Module.cpp" "${PROJECT_SOURCE_DIR}/source/Module.h"
	"${PROJECT_SOURCE_DIR}/source/Audio.cpp" "${PROJECT_SOURCE_DIR}/source/Audio.h"
	"${PROJECT_SOURCE_DIR}/source/Video.cpp" "${PROJECT_SOURCE_DIR}/source/Video.h"
	
	"${PROJECT_SOURCE_DIR}/source/nodeobs_module.h" "${PROJECT_SOURCE_DIR}/source/nodeobs_module.cpp"
	"${PROJECT_SOURCE_DIR}/source/nodeobs_api.h" "${PROJECT_SOURCE_DIR}/source/nodeobs_api.cpp"
	"${PROJECT_SOURCE_DIR}/source/nodeobs_audio.h" "${PROJECT_SOURCE_DIR}/source/nodeobs_audio.cpp"
	"${PROJECT_SOURCE_DIR}/source/nodeobs_audio_encoders.h" "${PROJECT_SOURCE_DIR}/source/nodeobs_audio_encoders.cpp"
	"${PROJECT_SOURCE_DIR}/source/nodeobs_autoconfig.h" "${PROJECT_SOURCE_DIR}/source/nodeobs_autoconfig.cpp"
	"${PROJECT_SOURCE_DIR}/source/nodeobs_common.h" "${PROJECT_SOURCE_DIR}/source/nodeobs_common.cpp"
	"${PROJECT_SOURCE_DIR}/source/nodeobs_display.h" "${PROJECT_SOURCE_DIR}/source/nodeobs_display.cpp"
	"${PROJECT_SOURCE_DIR}/source/nodeobs_event.h" "${PROJECT_SOURCE_DIR}/source/nodeobs_event.cpp"
	"${PROJECT_SOURCE_DIR}/source/nodeobs_service.h" "${PROJECT_SOURCE_DIR}/source/nodeobs_service.cpp"
	"${PROJECT_SOURCE_DIR}/source/nodeobs_settings.h" "${PROJECT_SOURCE_DIR}/source/nodeobs_settings.cpp"
	"${PROJECT_SOURCE_DIR}/source/nodeobs_obspp_manager.hpp" "${PROJECT_SOURCE_DIR}/source/nodeobs_obspp_manager.cpp"
	"${PROJECT_SOURCE_DIR}/source/nodeobs_obspp_index.hpp" "${PROJECT_SOURCE_DIR}/source/nodeobs_obspp_index.cpp"
	"${PROJECT_SOURCE_DIR}/source/nodeobs_content.h"
	"${PROJECT_SOURCE_DIR}/source/gs-limits.h"
	"${PROJECT_SOURCE_DIR}/source/gs-vertex.h" "${PROJECT_SOURCE_DIR}/source/gs-vertex.cpp"
	"${PROJECT_SOURCE_DIR}/source/gs-vertexbuffer.h" "${PROJECT_SOURCE_DIR}/source/gs-vertexbuffer.cpp"
	"${PROJECT_SOURCE_DIR}/source/util-memory.h" "${PROJECT_SOURCE_DIR}/source/util-memory.cpp"
)
SET(PROJECT_LIBRARIES
	${CMAKE_JS_LIB}
)
SET(PROJECT_INCLUDE_PATHS
	${CMAKE_JS_INC}
	"${CMAKE_SOURCE_DIR}/lib-streamlabs-ipc/include"
)
SET(PROJECT_DATA
	"resources/"
)
ADD_DEFINITIONS(-DBUILDING_NODE_EXTENSION)

#############################
# Building
#############################
ADD_LIBRARY(obs-studio-client SHARED ${PROJECT_SOURCE})
TARGET_LINK_LIBRARIES(obs-studio-client ${PROJECT_LIBRARIES})
TARGET_INCLUDE_DIRECTORIES(obs-studio-client PUBLIC ${PROJECT_INCLUDE_PATHS})
SET_TARGET_PROPERTIES(obs-studio-client PROPERTIES PREFIX "" SUFFIX ".node")

#############################
# Distribute
#############################
INSTALL(TARGETS obs-studio-client DESTINATION "${INSTALL_DIR}")
INSTALL(FILES $<TARGET_PDB_FILE:obs-studio-client> DESTINATION "${INSTALL_DIR}" OPTIONAL)
