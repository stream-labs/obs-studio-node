/******************************************************************************
    Copyright (C) 2016-2022 by Streamlabs (General Workings Inc)

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.

******************************************************************************/

#include "encoder.hpp"
#include "utility.hpp"
#include "properties.hpp"
#include "obs-property.hpp"

Napi::FunctionReference osn::Encoder::constructor;

Napi::Object osn::Encoder::Init(Napi::Env env, Napi::Object exports) {
	Napi::HandleScope scope(env);
	Napi::Function func =
		DefineClass(env,
		"Encoder",
		{
            StaticMethod("create", &osn::Encoder::Create),
            StaticMethod("types", &osn::Encoder::GetTypes),

            InstanceAccessor("name", &osn::Encoder::GetName, &osn::Encoder::SetName),
            InstanceAccessor("type", &osn::Encoder::GetType, nullptr),
            InstanceAccessor("active", &osn::Encoder::GetActive, nullptr),
            InstanceAccessor("id", &osn::Encoder::GetId, nullptr),
            InstanceAccessor("lastError", &osn::Encoder::GetLastError, nullptr),
            InstanceAccessor("properties", &osn::Encoder::GetProperties, nullptr),
            InstanceAccessor("settings", &osn::Encoder::GetSettings, nullptr),

            InstanceMethod("update", &osn::Encoder::Update),
            InstanceMethod("release", &osn::Encoder::Release),
		});
	exports.Set("Encoder", func);
	osn::Encoder::constructor = Napi::Persistent(func);
	osn::Encoder::constructor.SuppressDestruct();
	return exports;
}

osn::Encoder::Encoder(const Napi::CallbackInfo& info)
    : Napi::ObjectWrap<osn::Encoder>(info) {
    Napi::Env env = info.Env();
    Napi::HandleScope scope(env);
    int length = info.Length();

    if (length <= 0 || !info[0].IsNumber()) {
        Napi::TypeError::New(env, "Number expected").ThrowAsJavaScriptException();
        return;
    }

	this->uid = (uint64_t)info[0].ToNumber().Int64Value();
}

Napi::Value osn::Encoder::Create(const Napi::CallbackInfo& info) {
    std::string id = info[0].ToString().Utf8Value();
    std::string name = info[1].ToString().Utf8Value();

    // Settings
    std::string settings = "";

    if (info.Length() > 2) {
        Napi::Object json = info.Env().Global().Get("JSON").As<Napi::Object>();
        Napi::Function stringify = json.Get("stringify").As<Napi::Function>();
        Napi::String settingsObj = Napi::String::New(info.Env(), "");
        settings = stringify.Call(json, { settingsObj }).As<Napi::String>().Utf8Value();
    }

    auto conn = GetConnection(info);
	if (!conn)
		return info.Env().Undefined();

    std::vector<ipc::value> response =
        conn->call_synchronous_helper("Encoder", "Create", {
            ipc::value(id),
            ipc::value(name),
            ipc::value(settings)});

	if (!ValidateResponse(info, response))
		return info.Env().Undefined();

    auto instance =
        osn::Encoder::constructor.New({
            Napi::Number::New(info.Env(), response[1].value_union.ui64)
            });

    return instance;
}

Napi::Value osn::Encoder::GetTypes(const Napi::CallbackInfo& info) {
	auto conn = GetConnection(info);
	if (!conn)
		return info.Env().Undefined();
    
    int32_t type = -1;
    if (info.Length() > 0)
        type = info[0].ToNumber().Int32Value();

	std::vector<ipc::value> response =
		conn->call_synchronous_helper("Encoder", "GetTypes", {ipc::value(type)});

	if (!ValidateResponse(info, response))
		return info.Env().Undefined();

    Napi::Array types = Napi::Array::New(info.Env());
    for (int i = 1; i < response.size(); i++)
        types.Set(i - 1, Napi::String::New(info.Env(), response[i].value_str));
    
    return types;
}

Napi::Value osn::Encoder::GetName(const Napi::CallbackInfo& info) {
	auto conn = GetConnection(info);
	if (!conn)
		return info.Env().Undefined();

	std::vector<ipc::value> response =
		conn->call_synchronous_helper("Encoder", "GetName", {ipc::value(this->uid)});

	if (!ValidateResponse(info, response))
		return info.Env().Undefined();
    
    return Napi::String::New(info.Env(), response[1].value_str);
}

void osn::Encoder::SetName(const Napi::CallbackInfo& info, const Napi::Value &value) {
    std::string name = value.ToString().Utf8Value();

    auto conn = GetConnection(info);
	if (!conn)
		return;

    conn->call("Encoder", "SetName", {ipc::value(this->uid), ipc::value(name)});
}

Napi::Value osn::Encoder::GetType(const Napi::CallbackInfo& info) {
	auto conn = GetConnection(info);
	if (!conn)
		return info.Env().Undefined();

	std::vector<ipc::value> response =
		conn->call_synchronous_helper("Encoder", "GetType", {ipc::value(this->uid)});

	if (!ValidateResponse(info, response))
		return info.Env().Undefined();
    
    return Napi::Number::New(info.Env(), response[1].value_union.ui32);
}

Napi::Value osn::Encoder::GetActive(const Napi::CallbackInfo& info) {
	auto conn = GetConnection(info);
	if (!conn)
		return info.Env().Undefined();

	std::vector<ipc::value> response =
		conn->call_synchronous_helper("Encoder", "GetActive", {ipc::value(this->uid)});

	if (!ValidateResponse(info, response))
		return info.Env().Undefined();
    
    return Napi::Boolean::New(info.Env(), response[1].value_union.ui32);
}

Napi::Value osn::Encoder::GetId(const Napi::CallbackInfo& info) {
	auto conn = GetConnection(info);
	if (!conn)
		return info.Env().Undefined();

	std::vector<ipc::value> response =
		conn->call_synchronous_helper("Encoder", "GetId", {ipc::value(this->uid)});

	if (!ValidateResponse(info, response))
		return info.Env().Undefined();
    
    return Napi::String::New(info.Env(), response[1].value_str);
}

Napi::Value osn::Encoder::GetLastError(const Napi::CallbackInfo& info) {
	auto conn = GetConnection(info);
	if (!conn)
		return info.Env().Undefined();

	std::vector<ipc::value> response =
		conn->call_synchronous_helper("Encoder", "GetLastError", {ipc::value(this->uid)});

	if (!ValidateResponse(info, response))
		return info.Env().Undefined();
    
    return Napi::String::New(info.Env(), response[1].value_str);
}

void osn::Encoder::Release(const Napi::CallbackInfo& info) {
	auto conn = GetConnection(info);
	if (!conn)
		return;

	conn->call("Encoder", "Release", {});
}

void osn::Encoder::Update(const Napi::CallbackInfo& info) {
	Napi::Object jsonObj = info[0].ToObject();
	Napi::Object json = info.Env().Global().Get("JSON").As<Napi::Object>();
	Napi::Function stringify = json.Get("stringify").As<Napi::Function>();

	std::string jsondata = stringify.Call(json, { jsonObj }).As<Napi::String>();

    auto conn = GetConnection(info);
    if (!conn)
        return;

    conn->call(
        "Encoder",
        "Update",
        {ipc::value(this->uid), ipc::value(jsondata)});

}

Napi::Value osn::Encoder::GetProperties(const Napi::CallbackInfo& info) {
	auto conn = GetConnection(info);
	if (!conn)
		return info.Env().Undefined();

	std::vector<ipc::value> response =
		conn->call_synchronous_helper("Encoder", "GetProperties", {ipc::value(this->uid)});

	if (!ValidateResponse(info, response))
		return info.Env().Undefined();

	if (response.size() == 1)
		return info.Env().Undefined();

	osn::property_map_t pmap = osn::ProcessProperties(response, 1);

	std::shared_ptr<property_map_t> pSomeObject = std::make_shared<property_map_t>(pmap);
	auto prop_ptr = Napi::External<property_map_t>::New(info.Env(), pSomeObject.get());
	auto instance =
		osn::Properties::constructor.New({
			prop_ptr,
			Napi::Number::New(info.Env(), (uint32_t)this->uid)
			});

	return instance;
}


Napi::Value osn::Encoder::GetSettings(const Napi::CallbackInfo& info) {
	Napi::Object json = info.Env().Global().Get("JSON").As<Napi::Object>();
	Napi::Function parse = json.Get("parse").As<Napi::Function>();

	auto conn = GetConnection(info);
	if (!conn)
		return info.Env().Undefined();

	std::vector<ipc::value> response =
		conn->call_synchronous_helper("Encoder", "GetSettings", {ipc::value(this->uid)});

	if (!ValidateResponse(info, response))
		return info.Env().Undefined();

	Napi::String jsondata = Napi::String::New(info.Env(), response[1].value_str);
	Napi::Object jsonObj = parse.Call(json, {jsondata}).As<Napi::Object>();
	return jsonObj;
}
