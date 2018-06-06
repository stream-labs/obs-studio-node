#include <node.h>
#include <nan.h>
#include "utility-v8.hpp"

namespace autoConfig {
	static void GetListServer(const v8::FunctionCallbackInfo<v8::Value>& args);
	static void InitializeAutoConfig(const v8::FunctionCallbackInfo<v8::Value>& args);
	static void StartBandwidthTest(const v8::FunctionCallbackInfo<v8::Value>& args);
	static void StartStreamEncoderTest(const v8::FunctionCallbackInfo<v8::Value>& args);
	static void StartRecordingEncoderTest(const v8::FunctionCallbackInfo<v8::Value>& args);
	static void StartCheckSettings(const v8::FunctionCallbackInfo<v8::Value>& args);
	static void StartSetDefaultSettings(const v8::FunctionCallbackInfo<v8::Value>& args);
	static void StartSaveStreamSettings(const v8::FunctionCallbackInfo<v8::Value>& args);
	static void StartSaveSettings(const v8::FunctionCallbackInfo<v8::Value>& args);
	static void TerminateAutoConfig(const v8::FunctionCallbackInfo<v8::Value>& args);
}