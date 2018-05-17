#include <node.h>
#include <nan.h>
#include <thread>
#include <mutex>
#include "utility-v8.hpp"

std::thread query_worker;
bool query_worker_close = false;
std::mutex query_lock;
uint32_t sleepIntervalMS = 33;

struct SignalInfo {
	std::string outputType;
	std::string signal;
	int code;
	std::string errorMessage;
	void* param;
};

class Service;
typedef utilv8::CallbackData<SignalInfo, Service> ServiceCallback;
Service *serviceObject;

class Service : public Nan::ObjectWrap,
	public utilv8::InterfaceObject<Service>,
	public utilv8::ManagedObject<Service> {
	friend utilv8::InterfaceObject<Service>;
	friend utilv8::ManagedObject<Service>;
	friend utilv8::CallbackData<SignalInfo, Service>;

public:
	Service() {
		query_worker_close = false;
		query_worker = std::thread(std::bind(&Service::async_query, this));
	} ;
	~Service() {} ;
	void async_query();
	std::list<ServiceCallback*> callbacks;
	
	
	static void Callback(Service* volmeter, SignalInfo* item);
	static Nan::Persistent<v8::FunctionTemplate> prototype;
};

namespace service {
	static void OBS_service_resetAudioContext(const v8::FunctionCallbackInfo<v8::Value>& args);
	static void OBS_service_resetVideoContext(const v8::FunctionCallbackInfo<v8::Value>& args);
	static void OBS_service_createAudioEncoder(const v8::FunctionCallbackInfo<v8::Value>& args);
	static void OBS_service_createVideoStreamingEncoder(const v8::FunctionCallbackInfo<v8::Value>& args);
	static void OBS_service_createVideoRecordingEncoder(const v8::FunctionCallbackInfo<v8::Value>& args);
	static void OBS_service_createService(const v8::FunctionCallbackInfo<v8::Value>& args);
	static void OBS_service_createRecordingSettings(const v8::FunctionCallbackInfo<v8::Value>& args);
	static void OBS_service_createStreamingOutput(const v8::FunctionCallbackInfo<v8::Value>& args);
	static void OBS_service_createRecordingOutput(const v8::FunctionCallbackInfo<v8::Value>& args);
	static void OBS_service_startStreaming(const v8::FunctionCallbackInfo<v8::Value>& args);
	static void OBS_service_startRecording(const v8::FunctionCallbackInfo<v8::Value>& args);
	static void OBS_service_stopStreaming(const v8::FunctionCallbackInfo<v8::Value>& args);
	static void OBS_service_stopRecording(const v8::FunctionCallbackInfo<v8::Value>& args);
	static void OBS_service_associateAudioAndVideoToTheCurrentStreamingContext(const v8::FunctionCallbackInfo<v8::Value>& args);
	static void OBS_service_associateAudioAndVideoToTheCurrentRecordingContext(const v8::FunctionCallbackInfo<v8::Value>& args);
	static void OBS_service_associateAudioAndVideoEncodersToTheCurrentStreamingOutput(const v8::FunctionCallbackInfo<v8::Value>& args);
	static void OBS_service_associateAudioAndVideoEncodersToTheCurrentRecordingOutput(const v8::FunctionCallbackInfo<v8::Value>& args);
	static void OBS_service_setServiceToTheStreamingOutput(const v8::FunctionCallbackInfo<v8::Value>& args);
	static void OBS_service_setRecordingSettings(const v8::FunctionCallbackInfo<v8::Value>& args);
	static void OBS_service_connectOutputSignals(const v8::FunctionCallbackInfo<v8::Value>& args);
}