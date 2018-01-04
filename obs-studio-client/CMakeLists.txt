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
	"${PROJECT_SOURCE_DIR}/source/controller.cpp" "${PROJECT_SOURCE_DIR}/source/controller.hpp" "${PROJECT_SOURCE_DIR}/source/controller_js.cpp"
#	"${PROJECT_SOURCE_DIR}/source/osn-nodeobs.cpp" "${PROJECT_SOURCE_DIR}/source/osn-nodeobs.hpp"
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
