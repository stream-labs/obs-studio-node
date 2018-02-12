#include "obspp/obspp-service.hpp"
#include "Common.h"

#include <nan.h>

namespace osn
{

class Service : public Nan::ObjectWrap
{
public:
	static Nan::Persistent<v8::FunctionTemplate> prototype;

	typedef obs::weak<obs::service> weak_handle_t;
	typedef common::Object<Service, weak_handle_t> Object;
	friend Object;

	weak_handle_t handle;

	Service(obs::service input);
	Service(std::string id, std::string name, obs_data_t *settings, obs_data_t *hotkey);
	Service(std::string id, std::string name, obs_data_t *settings, bool is_private);

	static NAN_MODULE_INIT(Init);
	static NAN_METHOD(get_types);
	static NAN_METHOD(create);
	static NAN_METHOD(createPrivate);
	static NAN_METHOD(fromName);
	static NAN_METHOD(update);
	static NAN_METHOD(release);
	static NAN_METHOD(get_url);
	static NAN_METHOD(get_key);
	static NAN_METHOD(get_username);
	static NAN_METHOD(get_password);
	static NAN_METHOD(get_settings);
	static NAN_METHOD(get_properties);
	static NAN_METHOD(get_name);
	static NAN_METHOD(get_id);
};
}