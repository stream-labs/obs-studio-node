if "%ReleaseName%"=="debug" (
    cmake -H. ^
          -B"%SLBuildDirectory%" ^
          -G"%SLGenerator%" ^
          -A"x64" ^
          -DCMAKE_INSTALL_PREFIX="%SLFullDistributePath%\obs-studio-node" ^
          -DSTREAMLABS_BUILD=OFF ^
          -DNODEJS_NAME=%RuntimeName% ^
          -DNODEJS_URL=%RuntimeURL% ^
          -DNODEJS_VERSION="v%NodejsVersion%" ^
          -DLIBOBS_BUILD_TYPE="debug"
) else (
    cmake -H. ^
          -B"%SLBuildDirectory%" ^
          -G"%SLGenerator%" ^
          -A"x64" ^
          -DCMAKE_INSTALL_PREFIX="%SLFullDistributePath%\obs-studio-node" ^
          -DSTREAMLABS_BUILD=OFF ^
          -DNODEJS_NAME=%RuntimeName% ^
          -DNODEJS_URL=%RuntimeURL% ^
          -DNODEJS_VERSION="v%NodejsVersion%"
)
