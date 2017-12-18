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
)
SET(PROJECT_INCLUDE_PATHS
	${CMAKE_JS_INC}
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
SET_TARGET_PROPERTIES(obs-studio-client PROPERTIES PREFIX "${BINARY_PREFIX}" SUFFIX "${BINARY_SUFFIX}")

#############################
# Distribute
#############################
INSTALL(TARGETS obs-studio-client DESTINATION "${INSTALL_DIR}")
INSTALL(FILES $<TARGET_PDB_FILE:obs-studio-client> DESTINATION "${INSTALL_DIR}" OPTIONAL)
