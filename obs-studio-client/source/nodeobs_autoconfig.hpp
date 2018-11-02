#include <nan.h>
#include <node.h>
#include "utility-v8.hpp"

struct AutoConfigInfo
{
	std::string event;
	std::string description;
	double      percentage;
	void*       param;
};

class AutoConfig;
typedef utilv8::managed_callback<std::shared_ptr<AutoConfigInfo>> AutoConfigCallback;
AutoConfig*                                                       autoConfigObject;

class AutoConfig : public Nan::ObjectWrap,
                   public utilv8::InterfaceObject<AutoConfig>,
                   public utilv8::ManagedObject<AutoConfig>
{
	friend utilv8::InterfaceObject<AutoConfig>;
	friend utilv8::ManagedObject<AutoConfig>;
	friend utilv8::CallbackData<AutoConfigInfo, AutoConfig>;

	uint32_t sleepIntervalMS = 33;

	public:
	std::thread m_worker;
	bool        m_worker_stop = true;
	std::mutex  m_worker_lock;

	AutoConfigCallback* m_async_callback = nullptr;
	Nan::Callback       m_callback_function;

	AutoConfig(){};
	~AutoConfig();

	void start_async_runner();
	void stop_async_runner();
	void callback_handler(void* data, std::shared_ptr<AutoConfigInfo> item);
	void start_worker();
	void stop_worker();
	void worker();
	void set_keepalive(v8::Local<v8::Object>);

	std::list<AutoConfigCallback*> callbacks;
};

namespace autoConfig
{
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
} // namespace autoConfig
