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
#include <iostream>
#include <nan.h>
#include <fstream>
#include <locale>
#include <codecvt>

#pragma region Windows
#ifdef _WIN32
#include <windows.h>
#include <psapi.h>
#include <direct.h>
#include <wchar.h>

ProcessInfo spawn(std::string program, std::string commandLine, std::string workingDirectory) {
	PROCESS_INFORMATION m_win32_processInformation;
	STARTUPINFOW m_win32_startupInfo;

	// Buffers
	std::vector<wchar_t> programBuf;
	std::vector<wchar_t> commandLineBuf;
	std::vector<wchar_t> workingDirectoryBuf;

	// Convert to WideChar
	DWORD wr;
	programBuf.resize(MultiByteToWideChar(CP_UTF8, 0,
		program.data(), (int)program.size(),
		nullptr, 0) + 1);
	wr = MultiByteToWideChar(CP_UTF8, 0,
		program.data(), (int)program.size(),
		programBuf.data(), (int)programBuf.size());
	if (wr == 0) {
		// Conversion failed.
		DWORD errorCode = GetLastError();
		return {};
	}

	commandLineBuf.resize(MultiByteToWideChar(CP_UTF8, 0,
		commandLine.data(), (int)commandLine.size(),
		nullptr, 0) + 1);
	wr = MultiByteToWideChar(CP_UTF8, 0,
		commandLine.data(), (int)commandLine.size(),
		commandLineBuf.data(), (int)commandLineBuf.size());
	if (wr == 0) {
		// Conversion failed.
		DWORD errorCode = GetLastError();
		return {};
	}

	if (workingDirectory.length() > 1) {
		workingDirectoryBuf.resize(MultiByteToWideChar(CP_UTF8, 0,
			workingDirectory.data(), (int)workingDirectory.size(),
			nullptr, 0) + 1);
		if (workingDirectoryBuf.size() > 0) {
			wr = MultiByteToWideChar(CP_UTF8, 0,
				workingDirectory.data(), (int)workingDirectory.size(),
				workingDirectoryBuf.data(), (int)workingDirectoryBuf.size());
			if (wr == 0) {
				// Conversion failed.
				DWORD errorCode = GetLastError();
				return {};
			}
		}
	}

	// Build information
	memset(&m_win32_startupInfo, 0, sizeof(m_win32_startupInfo));
	memset(&m_win32_processInformation, 0, sizeof(m_win32_processInformation));

	// Launch process
	SetLastError(ERROR_SUCCESS);
	DWORD success = CreateProcessW(programBuf.data(), commandLineBuf.data(),
		nullptr, nullptr, false,
		CREATE_NO_WINDOW | DETACHED_PROCESS,
		nullptr,
		workingDirectory.length() > 0 ? workingDirectoryBuf.data() : nullptr,
		&m_win32_startupInfo,
		&m_win32_processInformation);
	if (!success) {
		DWORD error = GetLastError();
		return {};
	}

	return ProcessInfo(
		reinterpret_cast<uint64_t>(m_win32_processInformation.hProcess),
		static_cast<uint64_t>(m_win32_processInformation.dwProcessId)
	);
}

ProcessInfo open_process(uint64_t handle) {
	ProcessInfo pi;
	pi.handle = (uint64_t)OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_TERMINATE, false, (DWORD)handle);
	return pi;
}

bool close_process(ProcessInfo pi) {
	return !!CloseHandle((HANDLE)pi.handle);
}

/* Credit to this dude: https://stackoverflow.com/a/28056007/2255625 */
/* Basically Win32 doesn't provide a way to fetch the length of the
 * name of a module because it's a C-string */
std::string get_process_name(ProcessInfo pi) {
	LPTSTR lpBuffer = NULL;
	DWORD  dwBufferLength = 256;
	DWORD  dwReturnLength = 0;
	HANDLE hProcess = (HANDLE)pi.handle;
	HMODULE hModule;
	DWORD unused1;
	BOOL bSuccess;

	/* We rely on undocumented behavior here where
	 * enumerating a process' modules will provide
	 * the process HMODULE first every time. */
	bSuccess = EnumProcessModules(
		hProcess,
		&hModule, sizeof(hModule),
		&unused1
	);

	if (!bSuccess)
		return {};

	while (32768 >= dwBufferLength) {
		lpBuffer = new TCHAR[dwBufferLength];

		dwReturnLength =
			GetModuleFileName(hModule, lpBuffer, dwBufferLength);

		if (!dwReturnLength) {
			delete lpBuffer;
			return {};
		}

		if (dwBufferLength > dwReturnLength) {
			/* Notice that these are expensive
			 * but they do shrink the buffer to
			 * match the string */
#ifdef UNICODE
			std::string result(from_utf16_wide_to_utf8(lpBuffer));
			delete lpBuffer;
			return result;
#elif
			return lpBuffer;
#endif
		}

		delete lpBuffer;
		/* Increased buffer exponentially.
		 * Notice this will eventually match
		 * a perfect 32768 which is the max
		 * length of an NTFS file path. */
		dwBufferLength <<= 1;
	}

	/* Path too long */
	return {};
}

bool is_process_alive(ProcessInfo pinfo) {
	SetLastError(ERROR_SUCCESS);
	DWORD id = GetProcessId(reinterpret_cast<HANDLE>(pinfo.handle));
	if (id != static_cast<DWORD>(pinfo.id)) {
		return false;
	}
	DWORD errorCode = GetLastError();
	if (errorCode != ERROR_SUCCESS) {
		return false;
	}
	return true;
}

bool kill(ProcessInfo pinfo, uint32_t code, uint32_t& exitcode) {
	SetLastError(ERROR_SUCCESS);
	bool suc = TerminateProcess(reinterpret_cast<HANDLE>(pinfo.handle), code);
	DWORD errorCode = GetLastError();
	return (errorCode == ERROR_SUCCESS);
}

std::string get_working_directory() {
	std::vector<wchar_t> bufUTF16 = std::vector<wchar_t>(65535);
	std::vector<char> bufUTF8;

	_wgetcwd(bufUTF16.data(), bufUTF16.size());

	// Convert from Wide-char to UTF8
	DWORD bufferSize = WideCharToMultiByte(CP_UTF8, 0,
		bufUTF16.data(), bufUTF16.size(),
		nullptr, 0,
		NULL, NULL);
	bufUTF8.resize(bufferSize + 1);
	DWORD finalSize = WideCharToMultiByte(CP_UTF8, 0,
		bufUTF16.data(), bufUTF16.size(),
		bufUTF8.data(), bufUTF8.size(),
		NULL, NULL);
	if (finalSize == 0) {
		// Conversion failed.
		DWORD errorCode = GetLastError();
		return false;
	}

	return bufUTF8.data();
}

#endif
#pragma endregion Windows

static std::string serverBinaryPath = "";
static std::string serverWorkingPath = "";

Controller::Controller() {

}

Controller::~Controller() {
	disconnect();
}

std::shared_ptr<ipc::client> Controller::host(std::string uri) {
	if (m_isServer)
		return nullptr;

	// Store info
	std::string program = serverBinaryPath;
	std::string commandLine = '"' + serverBinaryPath + '"' + " " + uri;
	std::string workingDirectory = serverWorkingPath.length() > 0 ? serverWorkingPath : get_working_directory();

	// Test for existing process.
	std::string pid_path;

#ifdef _WIN32
	std::basic_string<TCHAR> tmp;
	tmp.resize(MAX_PATH + 1);
	DWORD tmp_len = GetTempPath(MAX_PATH + 1, &tmp[0]);

#ifdef UNICODE
	pid_path.assign(
		std::move(from_utf16_wide_to_utf8(tmp.data(), tmp_len)));
#endif

	pid_path.append("\\");
	pid_path.append(pid_path);
#endif
	std::ifstream pid_file;
	pid_file.open(pid_path);
	if (pid_file.is_open()) {
		// There is one 64-bit integer stored here.
		union {
			uint64_t pid;
			char pid_char[sizeof(uint64_t)];
		};
		pid_file.read(&pid_char[0], 8);

		ProcessInfo pi = open_process(pid);
		if (pi.handle != 0) {
			std::string name = get_process_name(pi);
			if (name == serverBinaryPath) {
				uint32_t dontcare = 0;
				kill(pi, -1, dontcare);
			}
			close_process(pi);
		}
		pid_file.close();
	}
	
	procId = spawn(serverBinaryPath, commandLine, workingDirectory);
	if (procId.id == 0) {
		return nullptr;
	}

	std::ofstream pid_file_w;
	pid_file_w.open(pid_path, std::ios::trunc);
	if (pid_file_w.is_open()) {
		// There is one 64-bit integer stored here.
		union {
			uint64_t pid;
			char pid_char[sizeof(uint64_t)];
		};
		pid = procId.id;
		pid_file_w.write(&pid_char[0], sizeof(uint64_t));
		pid_file_w.close();
	}

	// Connect
	std::shared_ptr<ipc::client> cl = connect(uri);
	if (!cl) { // Assume the server broke or was not allowed to run. 
		disconnect();
		uint32_t exitcode;
		kill(procId, 0, exitcode);
		return nullptr;
	}

	m_isServer = true;
	return m_connection;
}

std::shared_ptr<ipc::client> Controller::connect(std::string uri, std::chrono::nanoseconds timeout /*= std::chrono::seconds(5)*/) {
	if (m_isServer)
		return nullptr;

	if (m_connection)
		return nullptr;

	std::shared_ptr<ipc::client> cl;
	using std::chrono::high_resolution_clock;
	high_resolution_clock::time_point begin_time = high_resolution_clock::now();
	while ((high_resolution_clock::now() - begin_time) <= timeout) {
		try {
			cl = std::make_shared<ipc::client>(uri);
		} catch (...) {
			cl = nullptr;
		}
		if (cl)
			break;
		
		if (procId.handle != 0) {
			// We are the owner of the server, but m_isServer is false for now.
			if (!is_process_alive(procId)) {
				break;
			}
		}

		std::this_thread::sleep_for(std::chrono::milliseconds(100));
	}
	if (!cl) {
		return nullptr;
	}

	cl->authenticate();
	m_connection = cl;
	return m_connection;
}

void Controller::disconnect() {
	if (m_isServer) {
		// Attempt soft shut down.
		m_connection->call_synchronous_helper("System", "Shutdown", {});

		// Wait for process exit.
		auto wait_begin = std::chrono::high_resolution_clock::now();
		while (is_process_alive(procId)) {
			std::this_thread::sleep_for(std::chrono::milliseconds(1));
			auto dur = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now() - wait_begin);
			if (dur.count() >= 500) {
				break; // Failed.
			}
		}

		wait_begin = std::chrono::high_resolution_clock::now();
		while (is_process_alive(procId)) {
			uint32_t exitcode = 0;
			kill(procId, 0, exitcode);

			std::this_thread::sleep_for(std::chrono::milliseconds(1));
			auto dur = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now() - wait_begin);
			if (dur.count() >= 500) {
				break; // Failed.
			}
		}
		m_isServer = false;
	}
	m_connection = nullptr;
}

std::shared_ptr<ipc::client> Controller::GetConnection() {
	return m_connection;
}

#pragma region JavaScript
void js_setServerPath(const v8::FunctionCallbackInfo<v8::Value>& args) {
	auto isol = args.GetIsolate();
	if (args.Length() == 0) {
		isol->ThrowException(v8::Exception::SyntaxError(v8::String::NewFromUtf8(isol, "Too few arguments, usage: setServerPath(<string> binaryPath[, <string> workingDirectoryPath = get_working_directory()]).")));
		return;
	} else if (args.Length() > 2) {
		isol->ThrowException(v8::Exception::SyntaxError(v8::String::NewFromUtf8(isol, "Too many arguments.")));
		return;
	}

	if (!args[0]->IsString()) {
		isol->ThrowException(v8::Exception::TypeError(v8::String::NewFromUtf8(isol, "Argument 'binaryPath' must be of type 'String'.")));
		return;
	}
	serverBinaryPath = *v8::String::Utf8Value(args[0]);

	if (args.Length() == 2) {
		if (!args[1]->IsString()) {
			isol->ThrowException(v8::Exception::TypeError(v8::String::NewFromUtf8(isol, "Argument 'workingDirectoryPath' must be of type 'String'.")));
			return;
		}

		serverWorkingPath = *v8::String::Utf8Value(args[1]);
	} else {
		serverWorkingPath = get_working_directory();
	}

	return;
}

void js_connect(const v8::FunctionCallbackInfo<v8::Value>& args) {
	auto isol = args.GetIsolate();
	if (args.Length() == 0) {
		isol->ThrowException(v8::Exception::SyntaxError(Nan::New<v8::String>("Too few arguments, usage: connect(<string> uri).").ToLocalChecked()));
		return;
	} else if (args.Length() > 1) {
		isol->ThrowException(v8::Exception::SyntaxError(Nan::New<v8::String>("Too many arguments.").ToLocalChecked()));
		return;
	} else if (!args[0]->IsString()) {
		isol->ThrowException(v8::Exception::TypeError(Nan::New<v8::String>("Argument 'uri' must be of type 'String'.").ToLocalChecked()));
		return;
	}

	std::string uri = *v8::String::Utf8Value(args[0]);
	auto cl = Controller::GetInstance().connect(uri);
	if (!cl) {
		isol->ThrowException(v8::Exception::Error(Nan::New<v8::String>("Failed to connect.").ToLocalChecked()));
		return;
	}

	return;
}

void js_host(const v8::FunctionCallbackInfo<v8::Value>& args) {
	auto isol = args.GetIsolate();
	if (args.Length() == 0) {
		isol->ThrowException(v8::Exception::SyntaxError(Nan::New<v8::String>("Too few arguments, usage: host(uri).").ToLocalChecked()));
		return;
	} else if (args.Length() > 1) {
		isol->ThrowException(v8::Exception::SyntaxError(Nan::New<v8::String>("Too many arguments.").ToLocalChecked()));
		return;
	} else if (!args[0]->IsString()) {
		isol->ThrowException(v8::Exception::TypeError(Nan::New<v8::String>("Argument 'uri' must be of type 'String'.").ToLocalChecked()));
		return;
	}

	std::string uri = *v8::String::Utf8Value(args[0]);
	auto cl = Controller::GetInstance().host(uri);
	if (!cl) {
		isol->ThrowException(v8::Exception::Error(Nan::New<v8::String>("Failed to host and connect.").ToLocalChecked()));
		return;
	}

	return;
}

void js_connectOrHost(const v8::FunctionCallbackInfo<v8::Value>& args) {
	auto isol = args.GetIsolate();
	if (args.Length() == 0) {
		isol->ThrowException(v8::Exception::SyntaxError(Nan::New<v8::String>("Too few arguments, usage: connectOrHost(uri).").ToLocalChecked()));
		return;
	} else if (args.Length() > 1) {
		isol->ThrowException(v8::Exception::SyntaxError(Nan::New<v8::String>("Too many arguments.").ToLocalChecked()));
		return;
	} else if (!args[0]->IsString()) {
		isol->ThrowException(v8::Exception::TypeError(Nan::New<v8::String>("Argument 'uri' must be of type 'String'.").ToLocalChecked()));
		return;
	}

	std::string uri = *v8::String::Utf8Value(args[0]);
	auto cl = Controller::GetInstance().host(uri);
	if (!cl) {
		isol->ThrowException(v8::Exception::Error(Nan::New<v8::String>("IPC failed to connect or host.").ToLocalChecked()));
		return;
	}

	return;
}

void js_disconnect(const v8::FunctionCallbackInfo<v8::Value>& args) {
	Controller::GetInstance().disconnect();
}

INITIALIZER(js_ipc) {
	initializerFunctions.push([](v8::Local<v8::Object>& exports) {
		// IPC related functions will be under the IPC object.
		auto obj = v8::Object::New(exports->GetIsolate());
		NODE_SET_METHOD(obj, "setServerPath", js_setServerPath);
		NODE_SET_METHOD(obj, "connect", js_connect);
		NODE_SET_METHOD(obj, "host", js_host);
		NODE_SET_METHOD(obj, "connectOrHost", js_connectOrHost);
		NODE_SET_METHOD(obj, "disconnect", js_disconnect);
		// Temporary
		NODE_SET_METHOD(obj, "ConnectOrHost", js_connectOrHost);
		NODE_SET_METHOD(obj, "Disconnect", js_disconnect);
		exports->Set(v8::String::NewFromUtf8(exports->GetIsolate(), "IPC"), obj);
	});
}
#pragma endregion JavaScript
