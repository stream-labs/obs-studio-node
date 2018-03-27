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
#include "shared.hpp"
#include "utility.hpp"
#include <string>
#include <sstream>
#include <node.h>

#ifdef _WIN32
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>
#endif

#pragma region JavaScript

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

INITIALIZER(js_ipc) {
	initializerFunctions.push([](v8::Local<v8::Object>& exports) {
		// IPC related functions will be under the IPC object.
		auto obj = v8::Object::New(exports->GetIsolate());
		exports->Set(v8::String::NewFromUtf8(exports->GetIsolate(), "IPC"), obj);

		NODE_SET_METHOD(obj, "ConnectOrHost", ConnectOrHost);
		NODE_SET_METHOD(obj, "Disconnect", Disconnect);

	});
}
#pragma endregion JavaScript

Controller::Controller() {

}

Controller::~Controller() {
	Disconnect();
}

std::shared_ptr<ipc::client> Controller::Host(std::string uri) {
	if (m_isServer)
		return nullptr;

	// Concatenate command line.
	std::stringstream buf;
	buf << "obs-studio-server" << " " << uri;
	std::string cmdLine = buf.str();
	std::vector<char> cmdLineBuf(cmdLine.front(), cmdLine.back(), std::allocator<char>());

	// Build information
	memset(&m_win32_startupInfo, 0, sizeof(m_win32_startupInfo));
	memset(&m_win32_processInformation, 0, sizeof(m_win32_processInformation));

	// Launch process
	if (!CreateProcessA(NULL, cmdLineBuf.data(), NULL, NULL, false, 
		CREATE_NEW_CONSOLE, NULL, NULL, &m_win32_startupInfo, 
		&m_win32_processInformation)) {
		return nullptr;
	}
	m_isServer = true;
	
	// Try and connect.
	std::shared_ptr<ipc::client> cl;
	for (size_t n = 0; n < 5; n++) { // Attempt 5 times.
		if (!cl) cl = Connect(uri);
		if (cl) break;
	}
	
	if (!cl) { // Assume the server broke or was not allowed to run. 
		Disconnect();
		return nullptr;
	}
	
	m_connection = cl;
	return m_connection;
}

std::shared_ptr<ipc::client> Controller::Connect(std::string uri) {
	if (m_isServer)
		return nullptr;

	// Try and connect.
	std::shared_ptr<ipc::client> cl;
	for (size_t n = 0; n < 5; n++) { // Attempt 5 times.
		if (!cl) cl = Connect(uri);
		if (cl) break;
	}
	m_connection = cl;
	return m_connection;
}

void Controller::Disconnect() {
	if (m_isServer) {
	#ifdef _WIN32
		TerminateProcess(m_win32_processInformation.hProcess, -1);
	#endif
		m_isServer = false;
	}
	m_connection = nullptr;
}

std::shared_ptr<ipc::client> Controller::GetConnection() {
	return m_connection;
}
