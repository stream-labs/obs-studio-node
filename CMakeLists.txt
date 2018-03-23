CMAKE_MINIMUM_REQUIRED(VERSION 3.1)
PROJECT(obs-studio-node VERSION 0.0.1)

#############################
# Configuration
#############################

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake")

# Installation
SET(INSTALL_DIR  "${CMAKE_SOURCE_DIR}/distribute" CACHE PATH "Path to install files to")

#############################
# Projects
#############################

ADD_SUBDIRECTORY(lib-streamlabs-ipc)
ADD_SUBDIRECTORY(obs-studio-client)
ADD_SUBDIRECTORY(obs-studio-server)
