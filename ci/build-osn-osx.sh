set -x
set -e

cmake --build ${SLBuildDirectory} --target install --config ${BUILD_CONFIG}