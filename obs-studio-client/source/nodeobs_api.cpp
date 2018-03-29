#include "nodeobs_api.hpp"
#include "controller.hpp"
#include "utility-v8.hpp"

Nan::NAN_METHOD_RETURN_TYPE api::OBS_API_initAPI(Nan::NAN_METHOD_ARGS_TYPE info) {
	std::vector<ipc::value> ipc_args;

	std::string path;
	ASSERT_GET_VALUE(info[0], path);

	ipc_args.push_back(ipc::value(path));

	Controller::GetInstance().GetConnection()->
		call("API", "api::OBS_API_initAPI", ipc_args, NULL, NULL);
}

Nan::NAN_METHOD_RETURN_TYPE api::OBS_API_destroyOBS_API(Nan::NAN_METHOD_ARGS_TYPE info) {
	std::vector<ipc::value> ipc_args;

	Controller::GetInstance().GetConnection()->
		call("API", "OBS_API_destroyOBS_API", ipc_args, NULL, NULL);
} 

Nan::NAN_METHOD_RETURN_TYPE api::OBS_API_getPerformanceStatistics(Nan::NAN_METHOD_ARGS_TYPE info) {
	std::vector<ipc::value> ipc_args;

	Controller::GetInstance().GetConnection()->
		call("API", "OBS_API_getPerformanceStatistics", ipc_args, NULL, NULL);
}

Nan::NAN_METHOD_RETURN_TYPE api::OBS_API_getOBS_existingProfiles(Nan::NAN_METHOD_ARGS_TYPE info) {
	std::vector<ipc::value> ipc_args;

	Controller::GetInstance().GetConnection()->
		call("API", "OBS_API_getOBS_existingProfiles", ipc_args, NULL, NULL);
}

Nan::NAN_METHOD_RETURN_TYPE api::OBS_API_getOBS_existingSceneCollections(Nan::NAN_METHOD_ARGS_TYPE info) {
	std::vector<ipc::value> ipc_args;

	Controller::GetInstance().GetConnection()->
		call("API", "OBS_API_getOBS_existingSceneCollections", ipc_args, NULL, NULL);
}

Nan::NAN_METHOD_RETURN_TYPE api::OBS_API_isOBS_installed(Nan::NAN_METHOD_ARGS_TYPE info) {
	std::vector<ipc::value> ipc_args;

	Controller::GetInstance().GetConnection()->
		call("API", "OBS_API_isOBS_installed", ipc_args, NULL, NULL);
}

Nan::NAN_METHOD_RETURN_TYPE api::SetWorkingDirectory(Nan::NAN_METHOD_ARGS_TYPE info) {
	std::vector<ipc::value> ipc_args;

	std::string path;
	ASSERT_GET_VALUE(info[0], path);

	ipc_args.push_back(ipc::value(path));

	Controller::GetInstance().GetConnection()
		->call("API", "SetWorkingDirectory", ipc_args, NULL, NULL);
}
