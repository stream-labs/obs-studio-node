cmake ^
	-H. ^
	-B"%SLBuildDirectory%" ^
	-G"%SLGenerator%" ^
	-DCMAKE_INSTALL_PREFIX="%SLFullDistributePath%\obs-studio-node" ^
	-DNODEJS_NAME=%RuntimeName% ^
	-DNODEJS_URL=%RuntimeURL% ^
	-DNODEJS_VERSION=%RuntimeVersion%

cmake --build %SLBuildDirectory% --target install --config RelWithDebInfo