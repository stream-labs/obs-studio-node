#include <node.h>
#include <nan.h>
#include "utility-v8.hpp"

struct AutoConfigInfo {
	std::string event;
	std::string description;
	double percentage;
	void* param;
};

class AutoConfig;
typedef utilv8::CallbackData<AutoConfigInfo, AutoConfig> AutoConfigCallback;
AutoConfig *autoConfigObject;

class AutoConfig : public Nan::ObjectWrap,
	public utilv8::InterfaceObject<AutoConfig>,
	public utilv8::ManagedObject<AutoConfig> {
	friend utilv8::InterfaceObject<AutoConfig>;
	friend utilv8::ManagedObject<AutoConfig>;
	friend utilv8::CallbackData<AutoConfigInfo, AutoConfig>;

public:
	AutoConfig() {};
	~AutoConfig() {};

	AutoConfigCallback* callback;
	static void Callback(AutoConfig* volmeter, AutoConfigInfo* item);
	static Nan::Persistent<v8::FunctionTemplate> prototype;
};

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