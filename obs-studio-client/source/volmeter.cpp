// Client module for the OBS Studio node module.
// Copyright(C) 2017 Streamlabs (General Workings Inc)
// 
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
// 
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.See the
// GNU General Public License for more details.
// 
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110 - 1301, USA.

#include "volmeter.hpp"

Nan::Persistent<v8::FunctionTemplate> osn::VolMeter::prototype = Nan::Persistent<v8::FunctionTemplate>();

void osn::VolMeter::Register(Nan::ADDON_REGISTER_FUNCTION_ARGS_TYPE target) {
	auto fnctemplate = Nan::New<v8::FunctionTemplate>();
	fnctemplate->InstanceTemplate()->SetInternalFieldCount(1);
	fnctemplate->SetClassName(Nan::New<v8::String>("Volmeter").ToLocalChecked());

	// Class Template
	utilv8::SetTemplateField(fnctemplate, "create", Create);

	// Instance Template
	auto objtemplate = fnctemplate->PrototypeTemplate();
	utilv8::SetTemplateField(objtemplate, "create", Create);
	utilv8::SetTemplateField(objtemplate, "attach", Attach);
	utilv8::SetTemplateField(objtemplate, "detach", Detach);
	utilv8::SetTemplateField(objtemplate, "addCallback", AddCallback);
	utilv8::SetTemplateField(objtemplate, "removeCallback", RemoveCallback);

	// Stuff
	utilv8::SetObjectField(target, "Volmeter", fnctemplate->GetFunction());
	prototype.Reset(fnctemplate);
}

Nan::NAN_METHOD_RETURN_TYPE osn::VolMeter::Create(Nan::NAN_METHOD_ARGS_TYPE info) {
	//ASSERT_INFO_LENGTH_AT_LEAST(info, 1);

	//int fader_type;

	//ASSERT_GET_VALUE(info[0], fader_type);

	//Volmeter *binding = new Volmeter(static_cast<obs_fader_type>(fader_type));
	//auto object = Volmeter::Object::GenerateObject(binding);
	//info.GetReturnValue().Set(object);
}

Nan::NAN_METHOD_RETURN_TYPE osn::VolMeter::GetUpdateInterval(Nan::NAN_METHOD_ARGS_TYPE info) {
	//obs::volmeter &handle = Volmeter::Object::GetHandle(info.Holder());

	//info.GetReturnValue().Set(handle.interval());
}

Nan::NAN_METHOD_RETURN_TYPE osn::VolMeter::SetUpdateInterval(Nan::NAN_METHOD_ARGS_TYPE info) {
	//obs::volmeter &handle = Volmeter::Object::GetHandle(info.Holder());

	//int ms;

	//ASSERT_GET_VALUE(info[0], ms);

	//handle.interval(ms);
}

Nan::NAN_METHOD_RETURN_TYPE osn::VolMeter::Attach(Nan::NAN_METHOD_ARGS_TYPE info) {
	//obs::volmeter &handle = Volmeter::Object::GetHandle(info.Holder());

	//ASSERT_INFO_LENGTH(info, 1);

	//v8::Local<v8::Object> source_object;

	//ASSERT_GET_VALUE(info[0], source_object);

	//obs::source source = ISource::GetHandle(source_object);

	//handle.attach(source);
}

Nan::NAN_METHOD_RETURN_TYPE osn::VolMeter::Detach(Nan::NAN_METHOD_ARGS_TYPE info) {
	//obs::volmeter &handle = Volmeter::Object::GetHandle(info.Holder());

	//handle.detach();
}

Nan::NAN_METHOD_RETURN_TYPE osn::VolMeter::AddCallback(Nan::NAN_METHOD_ARGS_TYPE info) {
	//obs::volmeter &handle = Volmeter::Object::GetHandle(info.Holder());
	//Volmeter* binding = Nan::ObjectWrap::Unwrap<Volmeter>(info.Holder());

	//ASSERT_INFO_LENGTH(info, 1);

	//v8::Local<v8::Function> callback;
	//ASSERT_GET_VALUE(info[0], callback);

	//VolmeterCallback *cb_binding =
	//	new VolmeterCallback(binding, Volmeter::Callback, callback, 50);

	//cb_binding->user_data = new int;
	//*((int*)cb_binding->user_data) = binding->handle.nr_channels();

	//handle.add_callback(volmeter_cb_wrapper, cb_binding);

	//auto object = VolmeterCallback::Object::GenerateObject(cb_binding);
	//cb_binding->obj_ref.Reset(object);
	//info.GetReturnValue().Set(object);
}

Nan::NAN_METHOD_RETURN_TYPE osn::VolMeter::RemoveCallback(Nan::NAN_METHOD_ARGS_TYPE info) {
	//obs::volmeter &handle = Volmeter::Object::GetHandle(info.Holder());

	//v8::Local<v8::Object> cb_object;
	//ASSERT_GET_VALUE(info[0], cb_object);

	//VolmeterCallback *cb_binding =
	//	VolmeterCallback::Object::GetHandle(cb_object);

	//cb_binding->stopped = true;
	//cb_binding->obj_ref.Reset();

	//handle.remove_callback(volmeter_cb_wrapper, cb_binding);

	//delete cb_binding->user_data;
	//cb_binding->user_data = 0;

	// /* What's this? A memory leak? Nope! The GC will automagically
	//  * destroy the CallbackData structure when it becomes weak. We
	//  * just need to make sure its in an unusable state. */
}
