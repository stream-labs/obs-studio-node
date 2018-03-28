#include "nodeobs_api.hpp"
#include "controller.hpp"

void api::OBS_API_initAPI(const v8::FunctionCallbackInfo<v8::Value>& args) {
	std::vector<ipc::value> ipc_args;
	v8::String::Utf8Value path(args[0]);
	ipc_args.push_back(ipc::value(*path));

	Controller::GetInstance().GetConnection()->
		call("API", "api::OBS_API_initAPI", ipc_args, NULL, NULL);
}

void api::OBS_API_destroyOBS_API(const v8::FunctionCallbackInfo<v8::Value>& args) {
	std::vector<ipc::value> ipc_args;
	Controller::GetInstance().GetConnection()->
		call("API", "OBS_API_destroyOBS_API", ipc_args, NULL, NULL);
} 

void api::OBS_API_getPerformanceStatistics(const v8::FunctionCallbackInfo<v8::Value>& args) {
	std::vector<ipc::value> ipc_args;
	Controller::GetInstance().GetConnection()->
		call("API", "OBS_API_getPerformanceStatistics", ipc_args, NULL, NULL);
}

void api::OBS_API_getOBS_existingProfiles(const v8::FunctionCallbackInfo<v8::Value>& args) {
	std::vector<ipc::value> ipc_args;
	Controller::GetInstance().GetConnection()->
		call("API", "OBS_API_getOBS_existingProfiles", ipc_args, NULL, NULL);
}

void api::OBS_API_getOBS_existingSceneCollections(const v8::FunctionCallbackInfo<v8::Value>& args) {
	std::vector<ipc::value> ipc_args;
	Controller::GetInstance().GetConnection()->
		call("API", "OBS_API_getOBS_existingSceneCollections", ipc_args, NULL, NULL);
}

void api::OBS_API_isOBS_installed(const v8::FunctionCallbackInfo<v8::Value>& args) {
	std::vector<ipc::value> ipc_args;
	Controller::GetInstance().GetConnection()->
		call("API", "OBS_API_isOBS_installed", ipc_args, NULL, NULL);
}

void api::SetWorkingDirectory(const v8::FunctionCallbackInfo<v8::Value>& args) {
	std::vector<ipc::value> ipc_args;
	v8::String::Utf8Value path(args[0]);
	ipc_args.push_back(ipc::value(*path));

	Controller::GetInstance().GetConnection()
		->call("API", "SetWorkingDirectory", ipc_args, NULL, NULL);
}
