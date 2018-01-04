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

#include "controller.hpp"
#include "utility.hpp"
#include "shared.hpp"
#include <nan.h>

void ConnectOrHost(const v8::FunctionCallbackInfo<v8::Value>& args) {
	auto isol = args.GetIsolate();
	if (args.Length() == 0) {
		isol->ThrowException(v8::Exception::SyntaxError(v8::String::NewFromUtf8(isol, "Too few arguments, usage: ConnectOrHost(uri).")));
		return;
	} else if (args.Length() > 1) {
		isol->ThrowException(v8::Exception::SyntaxError(v8::String::NewFromUtf8(isol, "Too many arguments.")));
		return;
	} else if (!args[0]->IsString()) {
		isol->ThrowException(v8::Exception::TypeError(v8::String::NewFromUtf8(isol, "Argument 'uri' must be of type 'String'.")));
		return;
	}

	std::string uri = *v8::String::Utf8Value(args[0]);
	auto cl = Controller::GetInstance().Connect(uri);
	if (!cl) {
		cl = Controller::GetInstance().Host(uri);
		if (!cl) {
			isol->ThrowException(v8::Exception::Error(v8::String::NewFromUtf8(isol, "IPC failed to connect or host.")));
			return;
		}
	}

	return;
}

void Disconnect(const v8::FunctionCallbackInfo<v8::Value>& args) {
	Controller::GetInstance().Disconnect();
}

INITIALIZER(js_controller) {
	initializerFunctions.push([](v8::Local<v8::Object>& exports) {
		// IPC related functions will be under the IPC object.
		auto obj = v8::Object::New(exports->GetIsolate());
		exports->Set(v8::String::NewFromUtf8(exports->GetIsolate(), "IPC"), obj);

		NODE_SET_METHOD(obj, "ConnectOrHost", ConnectOrHost);
		NODE_SET_METHOD(obj, "Disconnect", Disconnect);

	});
}