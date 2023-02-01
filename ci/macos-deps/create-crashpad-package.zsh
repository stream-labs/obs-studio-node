#!/bin/zsh

# 1. Put the script to an empty folder
# 2. Build with Xcode 14 may fail. In this case:
#       - Download XCode 13.4.x from here: https://developer.apple.com/download/more/
#       - Switch to that version, for example: sudo xcode-select --switch /Applications/Xcode-13.4.app/
# 3. ./create-crashpad-package.zsh --architecture=arm64
# 4. ./create-crashpad-package.zsh --architecture=x86_64
# 5. Go to https://chromium.googlesource.com/crashpad/crashpad/+/HEAD/doc/developing.md

install_depot_tools() {
    # mkdir depot_tools
    # cd depot_tools
    # git clone https://chromium.googlesource.com/chromium/tools/depot_tools.git
    # export PATH=$PWD:$PATH
    # cd ..

    # Check if the folder exists
    if [ -d "depot_tools" ]
    then
        echo "### The 'depot_tools' folder exists. Please delete it or install 'depot_tools' manually and add to the PATH, then start the script again."
        exit 1
    fi
    # Clone
    echo "### git clone https://chromium.googlesource.com/chromium/tools/depot_tools.git"
    git clone https://chromium.googlesource.com/chromium/tools/depot_tools.git depot_tools
    # Check if failed
    if [ $? -ne 0 ]
    then
        echo "### Could not install 'depot_tools'. Please install it manually and add to the PATH, then start the script again."
        exit 1
    fi
    # Add to PATH
    export PATH=${PWD}/depot_tools:$PATH
}

download_crashpad() {
    # mkdir crashpad
    # cd crashpad
    # fetch crashpad

    if [ -d "${GCLIENT_ROOT_FOLDER}" ]
    then
        echo "### The sources will not be downloaded because '${GCLIENT_ROOT_FOLDER}' exists. Remove it manually if you want to re-download the sources!"
        if [ ! -d "${GIT_FOLDER}" ]
        then
            echo "### '${GIT_FOLDER}' does not exist. The sources are broken. Remove '${GCLIENT_ROOT_FOLDER}' and restart the script."
            exit 1
        fi
        return
    fi

    # Create the source parent folder
    mkdir "${GCLIENT_ROOT_FOLDER}"
    cd "${GCLIENT_ROOT_FOLDER}"

    # Fetch the source code
    echo "### fetch crashpad"
    fetch crashpad
    if [ $? -ne 0 ]
    then
        echo "### Could not fetch the crashpad source code."
        cd "${INITIAL_WORKING_FOLDER}"
        exit 1
    fi

    # Check if the source folder name is correct and the folder exists
    if [ ! -d "${GIT_FOLDER}" ]
    then
        echo "### The source folder '${GIT_FOLDER}' could not be found. Probably the 'fetch' command failed. Please check manually"
        cd "${INITIAL_WORKING_FOLDER}"
        exit 1
    fi
}

build_crashpad() {
    # cd crashpad
    # gn gen out/arm64 --args='target_os="mac" target_cpu="arm64" mac_deployment_target="10.15.0" is_debug=false' --ide=xcode
    # ninja -C out/arm64
    # gn gen out/x86_64 --args='target_os="mac" target_cpu="x64" mac_deployment_target="10.15.0" is_debug=false' --ide=xcode
    # ninja -C out/x86_64

    # Prepare build parameters
    case "${ARCHITECTURE}" in
        arm64)
            # mac_deployment_target=11.0 produces warnings, meanwhile the treat_warnings_as_errors=false option is not available.
            PARAMS="target_cpu=\"arm64\" mac_deployment_target=\"10.15.0\""
            ;;
        x86_64)
            PARAMS="target_cpu=\"x64\" mac_deployment_target=\"10.15.0\""
            ;;
        *)
            echo "Unknown architecture! Only 'arm64' or 'x86_64' is supported."
            exit 1
            ;;
    esac

    cd "${GIT_FOLDER}"

    # Check if the folder exists
    if [ -d "${BUILD_FOLDER_REL}" ]
    then
        echo "### The build folder '${BUILD_FOLDER}' exits. It will not be reconfigured. Remove it manually if you want to configure it from scratch."
    else
        # Configure
        echo "### gn gen ${BUILD_FOLDER_REL} ..."
        gn gen ${BUILD_FOLDER_REL} --args="${PARAMS} target_os=\"mac\" is_debug=false"
        if [ $? -ne 0 ]
        then
            echo "### Could not configure to build for ${ARCHITECTURE}"
            cd "${INITIAL_WORKING_FOLDER}"
            exit 1
        fi    
    fi

    # Build
    echo "### ninja -C ${BUILD_FOLDER_REL} ..."
    ninja -C ${BUILD_FOLDER_REL}
    if [ $? -ne 0 ]
    then
        echo "### Could not build for ${ARCHITECTURE}"
        cd "${INITIAL_WORKING_FOLDER}"
        exit 1
    fi

    cd "${INITIAL_WORKING_FOLDER}"
}

package_crashpad() {
    PACKAGE_FOLDER_NAME=crashpad-osx-$(git -C ${GIT_FOLDER} rev-parse --short HEAD)-b$(date +'%y%m%d')-${ARCHITECTURE}
    PACKAGE_FOLDER=${INITIAL_WORKING_FOLDER}/${PACKAGE_FOLDER_NAME}
    if [ -d "${PACKAGE_FOLDER}" ]
    then
        echo "### Package folder ${PACKAGE_FOLDER} exists. Remove it manually and start the script again."
        exit 1    
    fi
    if [ -e "${PACKAGE_FOLDER}.zip" ]
    then
        echo "### The package ${PACKAGE_FOLDER}.zip exists. Remove it manually and start the script again."
        exit 1    
    fi

    echo "### Creating the package folder: ${PACKAGE_FOLDER}"
    # Create the folders
    mkdir -p "${PACKAGE_FOLDER}/bin"
    mkdir -p "${PACKAGE_FOLDER}/include/build"
    mkdir -p "${PACKAGE_FOLDER}/lib"

    # Copy binaries
    cp "${BUILD_FOLDER}/crashpad_database_util" "${PACKAGE_FOLDER}/bin"
    cp "${BUILD_FOLDER}/crashpad_handler" "${PACKAGE_FOLDER}/bin"
    cp "${BUILD_FOLDER}/crashpad_http_upload" "${PACKAGE_FOLDER}/bin"

    # Copy includes
    cd ${GIT_FOLDER}
    find . -name '*.h' -not -path "./out/*"  | cpio -pdm "${PACKAGE_FOLDER}/include"    
    # one more file
    cp "${BUILD_FOLDER}/gen/build/chromeos_buildflags.h" "${PACKAGE_FOLDER}/include/build"

    # Copy libs
    cp "${BUILD_FOLDER}/obj/third_party/mini_chromium/mini_chromium/base/libbase.a" "${PACKAGE_FOLDER}/lib"
    cp "${BUILD_FOLDER}/obj/client/libclient.a" "${PACKAGE_FOLDER}/lib"
    cp "${BUILD_FOLDER}/obj/client/libcommon.a" "${PACKAGE_FOLDER}/lib"
    cp "${BUILD_FOLDER}/obj/util/libmig_output.a" "${PACKAGE_FOLDER}/lib"
    cp "${BUILD_FOLDER}/obj/util/libutil.a" "${PACKAGE_FOLDER}/lib"

    # Copy the script
    cp "${SCRIPT_PATH}" "${PACKAGE_FOLDER}"

    cd ${INITIAL_WORKING_FOLDER}

    # Zip everything
    echo "### Compressing ..."
    zip --quiet --recurse-paths ${PACKAGE_FOLDER_NAME}.zip ${PACKAGE_FOLDER_NAME}
}

# SCRIPT START

ARCHITECTURE=$(uname -m)

while [ $# -gt 0 ]; do
    case "$1" in
        --architecture=*)
            ARCHITECTURE="${1#*=}"
            ;;
        *)
            echo "Error: Invalid command line parameter. Please use --architecture=\"arm64\" or --architecture=\"x86_64\"."
            exit 1
    esac
    shift
done

INITIAL_WORKING_FOLDER=${PWD}
SCRIPT_PATH=$(realpath $0)
GCLIENT_ROOT_FOLDER_NAME=crashpad
GCLIENT_ROOT_FOLDER=${PWD}/${GCLIENT_ROOT_FOLDER_NAME}
GIT_FOLDER_NAME=crashpad
GIT_FOLDER=${GCLIENT_ROOT_FOLDER}/${GIT_FOLDER_NAME}
BUILD_FOLDER_NAME=${ARCHITECTURE}
BUILD_FOLDER_REL=out/${BUILD_FOLDER_NAME}
BUILD_FOLDER=${GIT_FOLDER}/${BUILD_FOLDER_REL}

# Check if git is available
if ! command -v git &> /dev/null
then
    echo "'git' could not be found. Please install 'git' and start the script again."
    exit 1
fi

# Check if depot_tools are available
if ! command -v gclient &> /dev/null
then
    # Check the subfolder
    export PATH=${PWD}/depot_tools:$PATH
    if ! command -v gclient &> /dev/null
    then
        echo "Could not find 'depot_tools'. Installing..."
        install_depot_tools
    fi
fi

# Downlaod the source code if it is necessary
download_crashpad

# Build 
build_crashpad

# Package
package_crashpad

