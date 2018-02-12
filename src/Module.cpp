#include "Module.h"

namespace osn
{

Module::Module(std::string path, std::string data_path)
	: handle(path, data_path)
{
}

NAN_METHOD(Module::create)
{
	ASSERT_INFO_LENGTH(info, 2);

	std::string path, data_path;

	ASSERT_GET_VALUE(info[0], path);
	ASSERT_GET_VALUE(info[1], data_path);

	Module *object = new Module(path, data_path);
	object->Wrap(info.This());
	info.GetReturnValue().Set(info.This());
}

NAN_MODULE_INIT(Module::Init)
{
	auto prototype = Nan::New<v8::FunctionTemplate>();
	prototype->SetClassName(FIELD_NAME("Module"));
	prototype->InstanceTemplate()->SetInternalFieldCount(1);

	common::SetObjectTemplateLazyAccessor(prototype->InstanceTemplate(), "filename",
	                                      get_filename);
	common::SetObjectTemplateLazyAccessor(prototype->InstanceTemplate(), "name",
	                                      get_name);
	common::SetObjectTemplateLazyAccessor(prototype->InstanceTemplate(), "author",
	                                      get_author);
	common::SetObjectTemplateLazyAccessor(prototype->InstanceTemplate(),
	                                      "description", get_description);
	common::SetObjectTemplateLazyAccessor(prototype->InstanceTemplate(), "binPath",
	                                      get_binPath);
	common::SetObjectTemplateLazyAccessor(prototype->InstanceTemplate(), "dataPath",
	                                      get_dataPath);
	common::SetObjectTemplateLazyAccessor(prototype->InstanceTemplate(), "status",
	                                      get_status);

	common::SetObjectTemplateField(prototype, "loadAll", loadAll);
	common::SetObjectTemplateField(prototype, "addPath", addPath);
	common::SetObjectTemplateField(prototype, "logLoaded", logLoaded);

	common::SetObjectField(target, "Module", prototype->GetFunction());
}

/* Prototype Methods */
NAN_METHOD(Module::addPath)
{
	ASSERT_INFO_LENGTH(info, 2);

	std::string bin_path, data_path;

	ASSERT_GET_VALUE(info[0], bin_path);
	ASSERT_GET_VALUE(info[1], data_path);

	obs::module::add_path(bin_path, data_path);
}

NAN_METHOD(Module::loadAll)
{
	obs::module::load_all();
}

NAN_METHOD(Module::logLoaded)
{
	obs::module::log_loaded();
}

/* Instance Methods */
NAN_METHOD(Module::initialize)
{
	obs::module &handle = Module::Object::GetHandle(info.Holder());

	handle.initialize();
}

NAN_METHOD(Module::get_filename)
{
	obs::module &handle = Module::Object::GetHandle(info.Holder());

	info.GetReturnValue().Set(common::ToValue(handle.file_name()));
}

NAN_METHOD(Module::get_name)
{
	obs::module &handle = Module::Object::GetHandle(info.Holder());

	info.GetReturnValue().Set(common::ToValue(handle.name()));
}

NAN_METHOD(Module::get_author)
{
	obs::module &handle = Module::Object::GetHandle(info.Holder());

	info.GetReturnValue().Set(common::ToValue(handle.author()));
}

NAN_METHOD(Module::get_description)
{
	obs::module &handle = Module::Object::GetHandle(info.Holder());

	info.GetReturnValue().Set(common::ToValue(handle.description()));
}

NAN_METHOD(Module::get_binPath)
{
	obs::module &handle = Module::Object::GetHandle(info.Holder());

	info.GetReturnValue().Set(common::ToValue(handle.binary_path()));
}

NAN_METHOD(Module::get_dataPath)
{
	obs::module &handle = Module::Object::GetHandle(info.Holder());

	info.GetReturnValue().Set(common::ToValue(handle.data_path()));
}

NAN_METHOD(Module::get_status)
{
	obs::module &handle = Module::Object::GetHandle(info.Holder());

	info.GetReturnValue().Set(handle.status());
}

}