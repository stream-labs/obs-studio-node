#include <node.h>
#include <nan.h>
#include <iostream>

namespace api {
	static void OBS_API_initAPI(const v8::FunctionCallbackInfo<v8::Value>& args);
	static void OBS_API_destroyOBS_API(const v8::FunctionCallbackInfo<v8::Value>& args);
	static void OBS_API_getPerformanceStatistics(const v8::FunctionCallbackInfo<v8::Value>& args);
	static void SetWorkingDirectory(const v8::FunctionCallbackInfo<v8::Value>& args);
}