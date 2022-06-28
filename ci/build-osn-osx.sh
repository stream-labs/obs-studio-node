set -x
set -e

cmake --build ${SLBUILDDIRECTORY} --target install --config ${BUILD_CONFIG}