/******************************************************************************
    Copyright (C) 2016-2019 by Streamlabs (General Workings Inc)

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

#include "fader.hpp"
#include <iterator>
#include <vector>
 #include "controller.hpp"
// #include "error.hpp"
// #include "isource.hpp"
 #include "shared.hpp"
#include <iostream>

Napi::FunctionReference osn::Fader::constructor;

Napi::Object osn::Fader::Init(Napi::Env env, Napi::Object exports) {
    Napi::HandleScope scope(env);
    Napi::Function func =
        DefineClass(env,
                    "Fader",
                    {
                        InstanceMethod("value", &osn::Fader::GetValue),
                        StaticMethod("create", &osn::Fader::Create),
                        StaticValue("test_static_value", Napi::Number::New(env, 2)),
                        InstanceValue("test_instance_value", Napi::Number::New(env, 3)),
                        InstanceMethod("registerCallback", &osn::Fader::RegisterCallback),
                        InstanceMethod("unregisterCallback", &osn::Fader::UnregisterCallback),
                    });
    constructor = Napi::Persistent(func);
    constructor.SuppressDestruct();

    exports.Set("Fader", func);
    return exports;
}

osn::Fader::Fader(const Napi::CallbackInfo& info)
    : Napi::ObjectWrap<osn::Fader>(info) {
    Napi::Env env = info.Env();
    Napi::HandleScope scope(env);
    int length = info.Length();

    if (length <= 0 || !info[0].IsNumber()) {
        Napi::TypeError::New(env, "Number expected").ThrowAsJavaScriptException();
        return;
    }

    this->uid = info[0].As<Napi::Number>().Int64Value();
}

inline void napi_inherits(napi_env env, napi_value ctor, napi_value super_ctor)
{
    napi_value global, global_object, set_proto, ctor_proto_prop, super_ctor_proto_prop;
    napi_value argv[2];

    napi_get_global(env, &global);
    napi_get_named_property(env, global, "Object", &global_object);
    napi_get_named_property(env, global_object, "setPrototypeOf", &set_proto);
    napi_get_named_property(env, ctor, "prototype", &ctor_proto_prop);
    napi_get_named_property(env, super_ctor, "prototype", &super_ctor_proto_prop);

    argv[0] = ctor_proto_prop;
    argv[1] = super_ctor_proto_prop;
    napi_call_function(env, global, set_proto, 2, argv, nullptr);

    argv[0] = ctor;
    argv[1] = super_ctor;
    napi_call_function(env, global, set_proto, 2, argv, nullptr);
}

Napi::FunctionReference osn::FaderSub::constructorSub;

Napi::Object osn::FaderSub::Init(Napi::Env env, Napi::Object exports) {
    Napi::HandleScope scope(env);
    Napi::Function func =
        DefineClass(env,
                    "FaderSub",
                    {
                        StaticValue("subvalue", Napi::Number::New(env, 4)),
                    });

    napi_inherits(env, func, osn::Fader::constructor.Value());
    exports.Set("FaderSub", func);
    constructorSub = Napi::Persistent(func);
    constructorSub.SuppressDestruct();
    return exports;
}

osn::FaderSub::FaderSub(const Napi::CallbackInfo& info)
    : Napi::ObjectWrap<osn::FaderSub>(info) {
    Napi::Env env = info.Env();
    Napi::HandleScope scope(env);
}

Napi::Value osn::Fader::GetValue(const Napi::CallbackInfo& info) {
std::cout << "GetValue 0" << std::endl;
  double uid = this->uid;

  return Napi::Number::New(info.Env(), uid);
}

Napi::Value osn::Fader::Create(const Napi::CallbackInfo& info) {
    // int length = info.Length();

    // if (length <= 0 || !info[0].IsNumber()) {
    //     Napi::TypeError::New(info.Env(), "Number expected").ThrowAsJavaScriptException();
    // }

    // int32_t fader_type = info[0].ToNumber().Int32Value();

	// // Validate Connection
	// auto conn = Controller::GetInstance().GetConnection();
	// if (!conn)
    //     Napi::TypeError::New(info.Env(), "IPC is not connected.").ThrowAsJavaScriptException();

	// // Call
	// std::vector<ipc::value> rval = conn->call_synchronous_helper(
	//     "Fader",
	//     "Create",
	//     {
	//         ipc::value(fader_type),
	//     });

	// if (!ValidateResponse(info, rval))
	// 	return info.Env().Undefined();

    // auto instance =
    //     osn::Fader::constructor.New({
    //         Napi::Number::New(info.Env(), rval[1].value_union.ui64)
    //         });
    // return instance;

    std::cout << "Create" << std::endl;

    auto instance =
        osn::Fader::constructor.New({
            Napi::Number::New(info.Env(), 2)
            });
    return instance;
}

Napi::Value osn::Fader::RegisterCallback(const Napi::CallbackInfo& info) {
    this->cb = info[0].As<Napi::Function>();
    FaderWorker* asyncWorker = new FaderWorker(cb);
    asyncWorker->Queue();
    FaderWorker* asyncWorker2 = new FaderWorker(cb);
    asyncWorker2->Queue();
    return Napi::Boolean::New(info.Env(), true);
}

Napi::Value osn::Fader::UnregisterCallback(const Napi::CallbackInfo& info) {
    // if (this->t_worker->joinable())
    //     this->t_worker->join();
    // std::this_thread::sleep_for(std::chrono::milliseconds(10000));
    return Napi::Boolean::New(info.Env(), true);
}

uint64_t osn::Fader::GetId()
{
	return this->uid;
}

// void osn::Fader::Register(Nan::ADDON_REGISTER_FUNCTION_ARGS_TYPE target)
// {
// 	auto fnctemplate = Nan::New<v8::FunctionTemplate>();
// 	fnctemplate->InstanceTemplate()->SetInternalFieldCount(1);
// 	fnctemplate->SetClassName(Nan::New<v8::String>("Fader").ToLocalChecked());

// 	// Class Template
// 	utilv8::SetTemplateField(fnctemplate, "create", Create);

// 	// Object Template
// 	auto objtemplate = fnctemplate->PrototypeTemplate();
// 	utilv8::SetTemplateAccessorProperty(objtemplate, "db", GetDeziBel, SetDezibel);
// 	utilv8::SetTemplateAccessorProperty(objtemplate, "deflection", GetDeflection, SetDeflection);
// 	utilv8::SetTemplateAccessorProperty(objtemplate, "mul", GetMultiplier, SetMultiplier);
// 	utilv8::SetTemplateField(objtemplate, "attach", Attach);
// 	utilv8::SetTemplateField(objtemplate, "detach", Detach);
// 	utilv8::SetTemplateField(objtemplate, "addCallback", AddCallback);
// 	utilv8::SetTemplateField(objtemplate, "removeCallback", RemoveCallback);

// 	// Stuff
// 	utilv8::SetObjectField(target, "Fader", fnctemplate->GetFunction(target->GetIsolate()->GetCurrentContext()).ToLocalChecked());
// 	prototype.Reset(fnctemplate);
// }

// void osn::Fader::Create(Nan::NAN_METHOD_ARGS_TYPE info)
// {
// 	int32_t fader_type;

// 	// Validate and retrieve parameters.
// 	ASSERT_INFO_LENGTH(info, 1);
// 	ASSERT_GET_VALUE(info[0], fader_type);

// 	// Validate Connection
// 	auto conn = Controller::GetInstance().GetConnection();
// 	if (!conn) {
// 		Nan::ThrowError("IPC is not connected.");
// 		return;
// 	}

// 	// Call
// 	std::vector<ipc::value> rval = conn->call_synchronous_helper(
// 	    "Fader",
// 	    "Create",
// 	    {
// 	        ipc::value(fader_type),
// 	    });

// 	if (!ValidateResponse(rval)) {
// 		return;
// 	}

// 	// Return created Object
// 	auto* newFader = new osn::Fader(rval[1].value_union.ui64);
// 	info.GetReturnValue().Set(Store(newFader));
// }

// void osn::Fader::GetDeziBel(Nan::NAN_METHOD_ARGS_TYPE info)
// {
// 	osn::Fader* fader;

// 	// Validate and retrieve parameters.
// 	ASSERT_INFO_LENGTH(info, 0);

// 	if (!Retrieve(info.This(), fader)) {
// 		return;
// 	}

// 	// Validate Connection
// 	auto conn = Controller::GetInstance().GetConnection();
// 	if (!conn) {
// 		Nan::ThrowError("IPC is not connected.");
// 		return;
// 	}

// 	// Call
// 	std::vector<ipc::value> rval = conn->call_synchronous_helper(
// 	    "Fader",
// 	    "GetDeziBel",
// 	    {
// 	        ipc::value(fader->uid),
// 	    });

// 	if (!ValidateResponse(rval)) {
// 		return;
// 	}

// 	// Return DeziBel Value
// 	info.GetReturnValue().Set(rval[1].value_union.fp32);
// }

// void osn::Fader::SetDezibel(Nan::NAN_METHOD_ARGS_TYPE info)
// {
// 	float_t     dezibel;
// 	osn::Fader* fader;

// 	// Validate and retrieve parameters.
// 	ASSERT_INFO_LENGTH(info, 1);
// 	ASSERT_GET_VALUE(info[0], dezibel);

// 	if (!Retrieve(info.This(), fader)) {
// 		return;
// 	}

// 	// Validate Connection
// 	auto conn = Controller::GetInstance().GetConnection();
// 	if (!conn) {
// 		Nan::ThrowError("IPC is not connected.");
// 		return;
// 	}

// 	// Call
// 	conn->call("Fader", "SetDeziBel", {ipc::value(fader->uid), ipc::value(dezibel)});
// }

// void osn::Fader::GetDeflection(Nan::NAN_METHOD_ARGS_TYPE info)
// {
// 	osn::Fader* fader;

// 	// Validate and retrieve parameters.
// 	ASSERT_INFO_LENGTH(info, 0);

// 	if (!Retrieve(info.This(), fader)) {
// 		return;
// 	}

// 	// Validate Connection
// 	auto conn = Controller::GetInstance().GetConnection();
// 	if (!conn) {
// 		Nan::ThrowError("IPC is not connected.");
// 		return;
// 	}

// 	// Call
// 	std::vector<ipc::value> rval = conn->call_synchronous_helper(
// 	    "Fader",
// 	    "GetDeflection",
// 	    {
// 	        ipc::value(fader->uid),
// 	    });

// 	if (!ValidateResponse(rval)) {
// 		return;
// 	}

// 	// Return DeziBel Value
// 	info.GetReturnValue().Set(rval[1].value_union.fp32);
// }

// void osn::Fader::SetDeflection(Nan::NAN_METHOD_ARGS_TYPE info)
// {
// 	float_t     dezibel;
// 	osn::Fader* fader;

// 	// Validate and retrieve parameters.
// 	ASSERT_INFO_LENGTH(info, 1);
// 	ASSERT_GET_VALUE(info[0], dezibel);

// 	if (!Retrieve(info.This(), fader)) {
// 		return;
// 	}

// 	// Validate Connection
// 	auto conn = Controller::GetInstance().GetConnection();
// 	if (!conn) {
// 		Nan::ThrowError("IPC is not connected.");
// 		return;
// 	}

// 	// Call
// 	conn->call("Fader", "SetDeflection", {ipc::value(fader->uid), ipc::value(dezibel)});
// }

// void osn::Fader::GetMultiplier(Nan::NAN_METHOD_ARGS_TYPE info)
// {
// 	osn::Fader* fader;

// 	// Validate and retrieve parameters.
// 	ASSERT_INFO_LENGTH(info, 0);

// 	if (!Retrieve(info.This(), fader)) {
// 		return;
// 	}

// 	// Validate Connection
// 	auto conn = Controller::GetInstance().GetConnection();
// 	if (!conn) {
// 		Nan::ThrowError("IPC is not connected.");
// 		return;
// 	}

// 	// Call
// 	std::vector<ipc::value> rval = conn->call_synchronous_helper(
// 	    "Fader",
// 	    "GetMultiplier",
// 	    {
// 	        ipc::value(fader->uid),
// 	    });

// 	if (!ValidateResponse(rval)) {
// 		return;
// 	}

// 	// Return DeziBel Value
// 	info.GetReturnValue().Set(rval[1].value_union.fp32);
// }

// void osn::Fader::SetMultiplier(Nan::NAN_METHOD_ARGS_TYPE info)
// {
// 	float_t     dezibel;
// 	osn::Fader* fader;

// 	// Validate and retrieve parameters.
// 	ASSERT_INFO_LENGTH(info, 1);
// 	ASSERT_GET_VALUE(info[0], dezibel);

// 	if (!Retrieve(info.This(), fader)) {
// 		return;
// 	}

// 	// Validate Connection
// 	auto conn = Controller::GetInstance().GetConnection();
// 	if (!conn) {
// 		Nan::ThrowError("IPC is not connected.");
// 		return;
// 	}

// 	// Call
// 	conn->call("Fader", "SetMultiplier", {ipc::value(fader->uid), ipc::value(dezibel)});
// }

// void osn::Fader::Attach(Nan::NAN_METHOD_ARGS_TYPE info)
// {
// 	osn::Fader*   fader;
// 	osn::ISource* source;

// 	// Validate and retrieve parameters.
// 	ASSERT_INFO_LENGTH(info, 1);

// 	if (!Retrieve(info.This(), fader)) {
// 		return;
// 	}

// 	v8::Local<v8::Object> sourceObj;
// 	ASSERT_GET_VALUE(info[0], sourceObj);
// 	if (!osn::ISource::Retrieve(sourceObj, source)) {
// 		return;
// 	}

// 	// Validate Connection
// 	auto conn = Controller::GetInstance().GetConnection();
// 	if (!conn) {
// 		Nan::ThrowError("IPC is not connected.");
// 		return;
// 	}

// 	// Call
// 	std::vector<ipc::value> rval =
// 	    conn->call_synchronous_helper("Fader", "Attach", {ipc::value(fader->uid), ipc::value(source->sourceId)});

// 	if (!ValidateResponse(rval)) {
// 		return;
// 	}
// }

// void osn::Fader::Detach(Nan::NAN_METHOD_ARGS_TYPE info)
// {
// 	osn::Fader* fader;

// 	// Validate and retrieve parameters.
// 	ASSERT_INFO_LENGTH(info, 0);

// 	if (!Retrieve(info.This(), fader)) {
// 		return;
// 	}

// 	// Validate Connection
// 	auto conn = Controller::GetInstance().GetConnection();
// 	if (!conn) {
// 		Nan::ThrowError("IPC is not connected.");
// 		return;
// 	}

// 	// Call
// 	std::vector<ipc::value> rval = conn->call_synchronous_helper("Fader", "Detach", {ipc::value(fader->uid)});

// 	if (!ValidateResponse(rval)) {
// 		return;
// 	}
// }

// void osn::Fader::AddCallback(Nan::NAN_METHOD_ARGS_TYPE info)
// {
// 	//obs::fader &handle = Fader::Object::GetHandle(info.Holder());
// 	//Fader* binding = Nan::ObjectWrap::Unwrap<Fader>(info.Holder());

// 	//ASSERT_INFO_LENGTH(info, 1);

// 	//v8::Local<v8::Function> callback;
// 	//ASSERT_GET_VALUE(info[0], callback);

// 	//FaderCallback *cb_binding =
// 	//	new FaderCallback(binding, Fader::Callback, callback);

// 	//handle.add_callback(fader_cb_wrapper, cb_binding);

// 	//auto object = FaderCallback::Object::GenerateObject(cb_binding);
// 	//cb_binding->obj_ref.Reset(object);
// 	//info.GetReturnValue().Set(object);
// }

// void osn::Fader::RemoveCallback(Nan::NAN_METHOD_ARGS_TYPE info)
// {
// 	//obs::fader &handle = Fader::Object::GetHandle(info.Holder());

// 	//v8::Local<v8::Object> cb_object;
// 	//ASSERT_GET_VALUE(info[0], cb_object);

// 	//FaderCallback *cb_binding =
// 	//	FaderCallback::Object::GetHandle(cb_object);

// 	//cb_binding->stopped = true;

// 	//handle.remove_callback(fader_cb_wrapper, cb_binding);
// 	//cb_binding->obj_ref.Reset();

// 	///* What's this? A memory leak? Nope! The GC will decide when
// 	//* and where to destroy the object. */
// }

// INITIALIZER(nodeobs_fader)
// {
// }