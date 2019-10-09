if "%BUILD_REASON%"=="PullRequest" (
    cmake -H. ^
          -B"%SLBuildDirectory%" ^
          -G"%SLGenerator%" ^
          -DCMAKE_INSTALL_PREFIX="%SLFullDistributePath%\obs-studio-node" ^
          -DSTREAMLABS_BUILD=OFF ^
          -DNODEJS_NAME=%RuntimeName% ^
          -DNODEJS_URL=%RuntimeURL% ^
          -DNODEJS_VERSION=%RuntimeVersion% ^
          -DOSN_LIBOBS_URL="https://obsstudios3.streamlabs.com/%LibOBSDebugArtifact%"
) else (
    cmake -H. ^
          -B"%SLBuildDirectory%" ^
          -G"%SLGenerator%" ^
          -DCMAKE_INSTALL_PREFIX="%SLFullDistributePath%\obs-studio-node" ^
          -DSTREAMLABS_BUILD=OFF ^
          -DNODEJS_NAME=%RuntimeName% ^
          -DNODEJS_URL=%RuntimeURL% ^
          -DNODEJS_VERSION=%RuntimeVersion%
)
