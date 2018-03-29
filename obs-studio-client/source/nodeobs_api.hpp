#include <node.h>
#include <nan.h>

namespace api {
	static Nan::NAN_METHOD_RETURN_TYPE OBS_API_initAPI(Nan::NAN_METHOD_ARGS_TYPE info);
	static Nan::NAN_METHOD_RETURN_TYPE OBS_API_destroyOBS_API(Nan::NAN_METHOD_ARGS_TYPE info);
	static Nan::NAN_METHOD_RETURN_TYPE OBS_API_getPerformanceStatistics(Nan::NAN_METHOD_ARGS_TYPE info);
	static Nan::NAN_METHOD_RETURN_TYPE OBS_API_getOBS_existingProfiles(Nan::NAN_METHOD_ARGS_TYPE info);
	static Nan::NAN_METHOD_RETURN_TYPE OBS_API_getOBS_existingSceneCollections(Nan::NAN_METHOD_ARGS_TYPE info);
	static Nan::NAN_METHOD_RETURN_TYPE OBS_API_isOBS_installed(Nan::NAN_METHOD_ARGS_TYPE info);
	static Nan::NAN_METHOD_RETURN_TYPE SetWorkingDirectory(Nan::NAN_METHOD_ARGS_TYPE info);
}