#include "IEncoder.h"
#include "Properties.h"
#include "Common.h"

namespace osn
{

obs::encoder IEncoder::GetHandle(v8::Local<v8::Object> object)
{
	IEncoderHandle *encoder = Nan::ObjectWrap::Unwrap<IEncoder>(object);
	return encoder->GetHandle();
}

NAN_MODULE_INIT(IEncoder::Init)
{
	auto locProto = Nan::New<v8::FunctionTemplate>();
	locProto->SetClassName(FIELD_NAME("Encoder"));
	common::SetObjectTemplateLazyAccessor(locProto->PrototypeTemplate(), "name",
	                                      get_name, set_name);
	common::SetObjectTemplateLazyAccessor(locProto->PrototypeTemplate(), "id",
	                                      get_id);
	common::SetObjectTemplateLazyAccessor(locProto->PrototypeTemplate(), "type",
	                                      get_type);
	common::SetObjectTemplateLazyAccessor(locProto->PrototypeTemplate(), "caps",
	                                      get_caps);
	common::SetObjectTemplateLazyAccessor(locProto->PrototypeTemplate(), "codec",
	                                      get_codec);
	common::SetObjectTemplateLazyAccessor(locProto->PrototypeTemplate(),
	                                      "properties", get_properties);
	common::SetObjectTemplateLazyAccessor(locProto->PrototypeTemplate(), "settings",
	                                      get_settings);
	common::SetObjectTemplateField(locProto->PrototypeTemplate(), "update", update);
	common::SetObjectTemplateField(locProto->PrototypeTemplate(), "release",
	                               release);

	common::SetObjectField(target, "Encoder", locProto->GetFunction());
	prototype.Reset(locProto);
}

NAN_METHOD(IEncoder::get_name)
{
	obs::encoder handle = IEncoder::GetHandle(info.Holder());

	info.GetReturnValue().Set(Nan::New(handle.name()).ToLocalChecked());
}

NAN_METHOD(IEncoder::set_name)
{
	obs::encoder handle = IEncoder::GetHandle(info.Holder());

	std::string name;

	ASSERT_GET_VALUE(info[0], name);

	handle.name(name);
}

NAN_METHOD(IEncoder::get_id)
{
	obs::encoder handle = IEncoder::GetHandle(info.Holder());

	info.GetReturnValue().Set(Nan::New(handle.id()).ToLocalChecked());
}

NAN_METHOD(IEncoder::get_caps)
{
	obs::encoder handle = IEncoder::GetHandle(info.Holder());

	info.GetReturnValue().Set(handle.caps());
}

NAN_METHOD(IEncoder::get_type)
{
	obs::encoder handle = IEncoder::GetHandle(info.Holder());

	info.GetReturnValue().Set(handle.type());
}

NAN_METHOD(IEncoder::get_codec)
{
	obs::encoder handle = IEncoder::GetHandle(info.Holder());

	info.GetReturnValue().Set(Nan::New(handle.codec()).ToLocalChecked());
}

NAN_METHOD(IEncoder::update)
{
	obs::encoder handle = IEncoder::GetHandle(info.Holder());

	obs_data_t *settings;

	ASSERT_GET_VALUE(info[0], settings);

	handle.update(settings);
}

NAN_METHOD(IEncoder::release)
{
	obs::encoder handle = IEncoder::GetHandle(info.Holder());

	handle.release();
}


NAN_METHOD(IEncoder::get_properties)
{
	obs::encoder handle = IEncoder::GetHandle(info.Holder());

	obs::properties props = handle.properties();

	if (props.status() != obs::properties::status_type::okay) {
		info.GetReturnValue().Set(Nan::Null());
		return;
	}

	Properties *bindings = new Properties(std::move(props));
	auto object = Properties::Object::GenerateObject(bindings);

	info.GetReturnValue().Set(object);
}

NAN_METHOD(IEncoder::get_settings)
{
	obs::encoder handle = IEncoder::GetHandle(info.Holder());

	obs_data_t *data = handle.settings();

	info.GetReturnValue().Set(common::ToValue(data));
}

Nan::Persistent<v8::FunctionTemplate> IEncoder::prototype =
      Nan::Persistent<v8::FunctionTemplate>();

}