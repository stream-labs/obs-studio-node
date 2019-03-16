PROJECT(obs_studio_client VERSION ${obs-studio-node_VERSION})

include(FetchContent)

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

if (STREAMLABS_BUILD)
	nodejs_init(
		URL https://atom.io/download/atom-shell
		NAME iojs
		VERSION v2.0.8
	)
else()
	nodejs_init()
endif()

# Nlohmann JSON (modern JSON for C++)
FetchContent_Declare(
  nlohmannjson
  GIT_REPOSITORY https://github.com/nlohmann/json
)

FetchContent_GetProperties(nlohmannjson)
if(NOT nlohmannjson_POPULATED)
  FetchContent_Populate(nlohmannjson)
endif()

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

	###### callback-manager ######
	"source/callback-manager.cpp"
	"source/callback-manager.hpp"
)

set(PROJECT_INCLUDE_PATHS
	"${nlohmannjson_SOURCE_DIR}/single_include"
)

if(WIN32)
	target_sources(obs_studio_client PUBLIC ${PROJECT_BINARY_DIR}/version.rc)
endif()

target_link_libraries(obs_studio_client lib-streamlabs-ipc)

target_include_directories(
	obs_studio_client
	PUBLIC
		"${CMAKE_SOURCE_DIR}/source/"
		"${CMAKE_SOURCE_DIR}/lib-streamlabs-ipc/include"
		${PROJECT_INCLUDE_PATHS}
)
 
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

install(
	TARGETS obs_studio_client
	RUNTIME DESTINATION "./" COMPONENT Runtime
	LIBRARY DESTINATION "./" COMPONENT Runtime
	ARCHIVE DESTINATION "./" COMPONENT Runtime	
)
