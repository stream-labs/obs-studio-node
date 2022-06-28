set -x

mkdir ${SLBuildDirectory}
cd ${SLBuildDirectory}
mkdir ${SLFullDistributePath}
cd ${SLFullDistributePath}
mkdir ${InstallPath}
cd ..

cmake .. \
-DCMAKE_OSX_DEPLOYMENT_TARGET=10.11 \
-DCMAKE_INSTALL_PREFIX=$PWD/${SLFullDistributePath}/${InstallPath} \
-DSTREAMLABS_BUILD=OFF \
-DNODEJS_NAME=${RUNTIMENAME} \
-DNODEJS_URL=${RUNTIMEURL} \
-DNODEJS_VERSION=v${ElectronVersion} \
-DLIBOBS_BUILD_TYPE=${RELEASE_NAME} \
-DCMAKE_BUILD_TYPE=${BUILD_CONFIG} \
-G Xcode

cd ..