echo $BuildConfig
echo $ReleaseName
echo $ElectronVersion
echo $RuntimeName
echo $SLFullDistributePath
echo $SLBuildDirectory

printenv

cmake ${SLBuildDirectory} \
-DCMAKE_INSTALL_PREFIX=${SLFullDistributePath}\obs-studio-node \
-DSTREAMLABS_BUILD=OFF \
-DNODEJS_NAME=${RuntimeName} \
-DNODEJS_URL=${RuntimeURL} \
-DNODEJS_VERSION=${ElectronVersion} \
-DLIBOBS_BUILD_TYPE=${ReleaseName} \
-DCMAKE_BUILD_TYPE=${BuildConfig}