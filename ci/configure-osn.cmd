if "%ReleaseName%"=="debug" (
    cmake -H. ^
          -B"%SLBuildDirectory%" ^
          -G"%SLGenerator%" ^
          -A"x64" ^
          -DCMAKE_INSTALL_PREFIX="%SLFullDistributePath%\obs-studio-node" ^
          -DSTREAMLABS_BUILD=OFF ^
          -DNODEJS_NAME=%RuntimeName% ^
          -DNODEJS_URL=%RuntimeURL% ^
          -DNODEJS_VERSION="v%ElectronVersion%" ^
          -DLIBOBS_BUILD_TYPE="debug" ^
          -DCMAKE_PREFIX_PATH="%CD%/%SLBuildDirectory%/libobs-src/cmake/"
) else (
    cmake -H. ^
          -B"%SLBuildDirectory%" ^
          -G"%SLGenerator%" ^
          -A"x64" ^
          -DCMAKE_INSTALL_PREFIX="%SLFullDistributePath%\obs-studio-node" ^
          -DSTREAMLABS_BUILD=OFF ^
          -DNODEJS_NAME=%RuntimeName% ^
          -DNODEJS_URL=%RuntimeURL% ^
          -DNODEJS_VERSION="v%ElectronVersion%" ^
          -DCMAKE_PREFIX_PATH="%CD%/%SLBuildDirectory%/libobs-src/cmake/"
)
