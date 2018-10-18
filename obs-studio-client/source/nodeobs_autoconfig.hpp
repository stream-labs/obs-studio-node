#include <nan.h>
#include <node.h>
#include "utility-v8.hpp"
#include "callback_manager.hpp"

struct AutoConfigInfo
{
	std::string event;
	std::string description;
	double      percentage;
	void*       param;
};

class AutoConfig : public Nan::ObjectWrap,
                   public utilv8::InterfaceObject<AutoConfig>,
                   public utilv8::ManagedObject<AutoConfig>
{
	friend utilv8::InterfaceObject<AutoConfig>;
	friend utilv8::ManagedObject<AutoConfig>;
	friend utilv8::CallbackData<AutoConfigInfo, AutoConfig>;

	private:
	CallbackManager<AutoConfigInfo> m_callback_manager;

	public:
	AutoConfig(){};
	~AutoConfig();

	CallbackManager<AutoConfigInfo>& get_callback_manager_ref();
	void callback_handler(void* data, std::shared_ptr<AutoConfigInfo> item, Nan::Callback& callback);
	void callback_update(CallbackManager<AutoConfigInfo>::DataCallback* dataCallback);
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
