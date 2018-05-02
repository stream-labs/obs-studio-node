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

#include "fader.hpp"

void osn::Fader::Register(Nan::ADDON_REGISTER_FUNCTION_ARGS_TYPE target) {

}

Nan::NAN_METHOD_RETURN_TYPE osn::Fader::Create(Nan::NAN_METHOD_ARGS_TYPE info) {
	//ASSERT_INFO_LENGTH_AT_LEAST(info, 1);

	//int fader_type;

	//ASSERT_GET_VALUE(info[0], fader_type);

	//Fader *binding = new Fader(static_cast<obs_fader_type>(fader_type));
	//auto object = Fader::Object::GenerateObject(binding);
	//info.GetReturnValue().Set(object);
}

Nan::NAN_METHOD_RETURN_TYPE osn::Fader::GetDeziBel(Nan::NAN_METHOD_ARGS_TYPE info) {
	//obs::fader &handle = Fader::Object::GetHandle(info.Holder());

	//info.GetReturnValue().Set(handle.db());
}

Nan::NAN_METHOD_RETURN_TYPE osn::Fader::SetDezibel(Nan::NAN_METHOD_ARGS_TYPE info) {
	//obs::fader &handle = Fader::Object::GetHandle(info.Holder());

	//float db;

	//ASSERT_GET_VALUE(info[0], db);

	//handle.db(db);
}

Nan::NAN_METHOD_RETURN_TYPE osn::Fader::GetDeflection(Nan::NAN_METHOD_ARGS_TYPE info) {
	//obs::fader &handle = Fader::Object::GetHandle(info.Holder());

	//info.GetReturnValue().Set(handle.deflection());
}

Nan::NAN_METHOD_RETURN_TYPE osn::Fader::SetDeflection(Nan::NAN_METHOD_ARGS_TYPE info) {
	//obs::fader &handle = Fader::Object::GetHandle(info.Holder());

	//float def;

	//ASSERT_GET_VALUE(info[0], def);

	//handle.deflection(def);
}

Nan::NAN_METHOD_RETURN_TYPE osn::Fader::GetMul(Nan::NAN_METHOD_ARGS_TYPE info) {
	//obs::fader &handle = Fader::Object::GetHandle(info.Holder());

	//info.GetReturnValue().Set(handle.mul());
}

Nan::NAN_METHOD_RETURN_TYPE osn::Fader::SetMul(Nan::NAN_METHOD_ARGS_TYPE info) {
	//obs::fader &handle = Fader::Object::GetHandle(info.Holder());

	//float mul;

	//ASSERT_GET_VALUE(info[0], mul);

	//handle.mul(mul);
}

Nan::NAN_METHOD_RETURN_TYPE osn::Fader::Attach(Nan::NAN_METHOD_ARGS_TYPE info) {
	//obs::fader &handle = Fader::Object::GetHandle(info.Holder());

	//ASSERT_INFO_LENGTH(info, 1);

	//v8::Local<v8::Object> source_object;

	//ASSERT_GET_VALUE(info[0], source_object);

	//obs::source source = ISource::GetHandle(source_object);

	//handle.attach(source);
}

Nan::NAN_METHOD_RETURN_TYPE osn::Fader::Detach(Nan::NAN_METHOD_ARGS_TYPE info) {
	//obs::fader &handle = Fader::Object::GetHandle(info.Holder());

	//handle.detach();
}

Nan::NAN_METHOD_RETURN_TYPE osn::Fader::AddCallback(Nan::NAN_METHOD_ARGS_TYPE info) {
	//obs::fader &handle = Fader::Object::GetHandle(info.Holder());
	//Fader* binding = Nan::ObjectWrap::Unwrap<Fader>(info.Holder());

	//ASSERT_INFO_LENGTH(info, 1);

	//v8::Local<v8::Function> callback;
	//ASSERT_GET_VALUE(info[0], callback);

	//FaderCallback *cb_binding =
	//	new FaderCallback(binding, Fader::Callback, callback);

	//handle.add_callback(fader_cb_wrapper, cb_binding);

	//auto object = FaderCallback::Object::GenerateObject(cb_binding);
	//cb_binding->obj_ref.Reset(object);
	//info.GetReturnValue().Set(object);
}

Nan::NAN_METHOD_RETURN_TYPE osn::Fader::RemoveCallback(Nan::NAN_METHOD_ARGS_TYPE info) {
	//obs::fader &handle = Fader::Object::GetHandle(info.Holder());

	//v8::Local<v8::Object> cb_object;
	//ASSERT_GET_VALUE(info[0], cb_object);

	//FaderCallback *cb_binding =
	//	FaderCallback::Object::GetHandle(cb_object);

	//cb_binding->stopped = true;

	//handle.remove_callback(fader_cb_wrapper, cb_binding);
	//cb_binding->obj_ref.Reset();

	///* What's this? A memory leak? Nope! The GC will decide when
	//* and where to destroy the object. */
}
