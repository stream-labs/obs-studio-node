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
#include <codecvt>
#include <fstream>
#include <locale>
#include <nan.h>
#include <sstream>
#include <string>
#include "shared.hpp"
#include "utility.hpp"

static std::string serverBinaryPath  = "";
static std::string serverWorkingPath = "";

#ifdef _WIN32
#include <direct.h>
#include <psapi.h>
#include <wchar.h>
#include <windows.h>

ProcessInfo spawn(const std::string& program, const std::string& commandLine, const std::string& workingDirectory)
{
	PROCESS_INFORMATION m_win32_processInformation = {0};
	STARTUPINFOW        m_win32_startupInfo        = {0};

	const std::wstring utfProgram(from_utf8_to_utf16_wide(program.c_str()));

	std::wstring utfCommandLine(from_utf8_to_utf16_wide(commandLine.c_str()));

	const std::wstring utfWorkingDir(from_utf8_to_utf16_wide(workingDirectory.c_str()));

	BOOL success = CreateProcessW(
	    utfProgram.c_str(),
	    /* Note that C++11 says this is fine since an
		 * std::string is guaranteed to be null-terminated. */
	    &utfCommandLine[0],
	    NULL,
	    NULL,
	    FALSE,
	    CREATE_NO_WINDOW | DETACHED_PROCESS,
	    NULL,
	    utfWorkingDir.empty() ? NULL : utfWorkingDir.c_str(),
	    &m_win32_startupInfo,
	    &m_win32_processInformation);

	if (!success)
		return {};

	return ProcessInfo(
	    reinterpret_cast<uint64_t>(m_win32_processInformation.hProcess),
	    static_cast<uint64_t>(m_win32_processInformation.dwProcessId));
}

ProcessInfo open_process(uint64_t handle)
{
	ProcessInfo pi;
	DWORD       flags = PROCESS_QUERY_INFORMATION | PROCESS_TERMINATE | PROCESS_VM_READ;

	pi.handle = (uint64_t)OpenProcess(flags, false, (DWORD)handle);
	return pi;
}

bool close_process(ProcessInfo pi)
{
	return !!CloseHandle((HANDLE)pi.handle);
}

/* Credit to this dude: https://stackoverflow.com/a/28056007/2255625 */
/* Basically Win32 doesn't provide a way to fetch the length of the
 * name of a module because it's a C-string */
std::string get_process_name(ProcessInfo pi)
{
	LPWSTR  lpBuffer       = NULL;
	DWORD   dwBufferLength = 256;
	HANDLE  hProcess       = (HANDLE)pi.handle;
	HMODULE hModule;
	DWORD   unused1;
	BOOL    bSuccess;

	/* We rely on undocumented behavior here where
	 * enumerating a process' modules will provide
	 * the process HMODULE first every time. */
	bSuccess = EnumProcessModules(hProcess, &hModule, sizeof(hModule), &unused1);

	if (!bSuccess)
		return {};

	while (32768 >= dwBufferLength) {
		std::wstring lpBuffer(dwBufferLength, wchar_t());

		DWORD dwReturnLength = GetModuleFileNameExW(hProcess, hModule, &lpBuffer[0], dwBufferLength);

		if (!dwReturnLength)
			return {};

		if (dwBufferLength <= dwReturnLength) {
			/* Increased buffer exponentially.
			 * Notice this will eventually match
			 * a perfect 32768 which is the max
			 * length of an NTFS file path. */
			dwBufferLength <<= 1;
			continue;
		}

		/* Notice that these are expensive
		 * but they do shrink the buffer to
		 * match the string */
		return from_utf16_wide_to_utf8(lpBuffer.data());
	}

	/* Path too long */
	return {};
}

bool is_process_alive(ProcessInfo pinfo)
{
	DWORD status;

	if (GetExitCodeProcess(reinterpret_cast<HANDLE>(pinfo.handle), &status) && status ==  static_cast<uint64_t>(259))
		return true;

	return false;
}

bool kill(ProcessInfo pinfo, uint32_t code, uint32_t& exitcode)
{
	return TerminateProcess(reinterpret_cast<HANDLE>(pinfo.handle), code);
}

std::string get_working_directory()
{
	DWORD        dwRequiredSize = GetCurrentDirectoryW(0, NULL);
	std::wstring lpBuffer(dwRequiredSize, wchar_t());

	GetCurrentDirectoryW(dwRequiredSize, &lpBuffer[0]);

	return from_utf16_wide_to_utf8(lpBuffer.data());
}

std::string get_temp_directory()
{
	constexpr DWORD tmp_size = MAX_PATH + 1;
	std::wstring    tmp(tmp_size, wchar_t());
	GetTempPathW(tmp_size, &tmp[0]);

	/* Here we resize an in-use string and then re-use it.
	 * Note this is only okay because the long path name
	 * will always be equal to or larger than the short
	 * path name */
	DWORD tmp_len = GetLongPathNameW(&tmp[0], NULL, 0);
	tmp.resize(tmp_len);

	/* Note that this isn't a hack to use the same buffer,
	 * it's explicitly meant to be used this way per MSDN. */
	GetLongPathNameW(&tmp[0], &tmp[0], tmp_len);

	return from_utf16_wide_to_utf8(tmp.data());
}

std::fstream open_file(std::string& file_path, std::fstream::openmode mode)
{
	return std::fstream(from_utf8_to_utf16_wide(file_path.c_str()), mode);
}

static void check_pid_file(std::string& pid_path)
{
	std::fstream::openmode mode = std::fstream::in | std::fstream::binary;

	std::fstream pid_file(open_file(pid_path, mode));

	union
	{
		uint64_t pid;
		char     pid_char[sizeof(uint64_t)];
	};

	if (!pid_file)
		return;

	pid_file.read(&pid_char[0], 8);

	ProcessInfo pi = open_process(pid);

	if (pi.handle == 0)
		return;

	std::string name = get_process_name(pi);

	/* FIXME TODO I don't like globals */
	if (name == serverBinaryPath) {
		uint32_t dontcare = 0;
		kill(pi, -1, dontcare);
	}

	close_process(pi);
}

static void write_pid_file(std::string& pid_path, uint64_t pid)
{
	std::fstream::openmode mode = std::fstream::out | std::fstream::binary | std::fstream::trunc;

	std::fstream pid_file(open_file(pid_path, mode));

	if (!pid_file)
		return;

	pid_file.write(reinterpret_cast<char*>(&pid), sizeof(pid));
}

#endif

Controller::Controller() {}

Controller::~Controller()
{
	disconnect();
}

std::shared_ptr<ipc::client> Controller::host(const std::string& uri)
{
	if (m_isServer)
		return nullptr;

	std::stringstream commandLine;
	commandLine << "\"" << serverBinaryPath << "\""
	            << " " << uri;

	std::string workingDirectory;
#ifdef WIN32
	if (serverWorkingPath.empty())
		workingDirectory = get_working_directory();
	else
		workingDirectory = serverWorkingPath;

	// Test for existing process.
	std::string pid_path(get_temp_directory());
	pid_path.append("server.pid");

	check_pid_file(pid_path);

	procId = spawn(serverBinaryPath, commandLine.str(), workingDirectory);
	if (procId.id == 0) {
		return nullptr;
	}

	write_pid_file(pid_path, procId.id);

	// Connect
	std::shared_ptr<ipc::client> cl = connect(uri);
	if (!cl) { // Assume the server broke or was not allowed to run.
		disconnect();
		uint32_t exitcode;
		kill(procId, 0, exitcode);
		return nullptr;
	}
#endif
	m_isServer = true;
	return m_connection;
}

std::shared_ptr<ipc::client> Controller::connect(
    const std::string& uri)
{
	if (m_isServer)
		return nullptr;

	if (m_connection)
		return nullptr;

	std::shared_ptr<ipc::client> cl;
	using std::chrono::high_resolution_clock;
	high_resolution_clock::time_point begin_time = high_resolution_clock::now();
	while (!cl) {
		try {
			cl = std::make_shared<ipc::client>(uri);
		} catch (...) {
			cl = nullptr;
		}
		if (cl)
			break;

		if (procId.handle != 0) {
#ifdef WIN32
			// We are the owner of the server, but m_isServer is false for now.
			if (!is_process_alive(procId)) {
				break;
			}
#endif
		}

		std::this_thread::sleep_for(std::chrono::milliseconds(100));
	}
	if (!cl) {
		return nullptr;
	}

	m_connection = cl;
	return m_connection;
}

void Controller::disconnect()
{
	if (m_isServer) {
		m_connection->call_synchronous_helper("System", "Shutdown", {});
		m_isServer = false;
	}
	m_connection = nullptr;
}

std::shared_ptr<ipc::client> Controller::GetConnection()
{
	return m_connection;
}

void js_setServerPath(const v8::FunctionCallbackInfo<v8::Value>& args)
{
	auto isol = args.GetIsolate();
	if (args.Length() == 0) {
		isol->ThrowException(v8::Exception::SyntaxError(v8::String::NewFromUtf8(
		    isol,
		    "Too few arguments, usage: setServerPath(<string> binaryPath[, <string> workingDirectoryPath = "
		    "get_working_directory()]).")));
		return;
	} else if (args.Length() > 2) {
		isol->ThrowException(v8::Exception::SyntaxError(v8::String::NewFromUtf8(isol, "Too many arguments.")));
		return;
	}

	if (!args[0]->IsString()) {
		isol->ThrowException(
		    v8::Exception::TypeError(v8::String::NewFromUtf8(isol, "Argument 'binaryPath' must be of type 'String'.")));
		return;
	}
	serverBinaryPath = *v8::String::Utf8Value(args[0]);

	if (args.Length() == 2) {
		if (!args[1]->IsString()) {
			isol->ThrowException(v8::Exception::TypeError(
			    v8::String::NewFromUtf8(isol, "Argument 'workingDirectoryPath' must be of type 'String'.")));
			return;
		}

		serverWorkingPath = *v8::String::Utf8Value(args[1]);
	} else {
#ifdef WIN32
		serverWorkingPath = get_working_directory();
#endif
    }

	return;
}

void js_connect(const v8::FunctionCallbackInfo<v8::Value>& args)
{
	auto isol = args.GetIsolate();
	if (args.Length() == 0) {
		isol->ThrowException(v8::Exception::SyntaxError(
		    Nan::New<v8::String>("Too few arguments, usage: connect(<string> uri).").ToLocalChecked()));
		return;
	} else if (args.Length() > 1) {
		isol->ThrowException(v8::Exception::SyntaxError(Nan::New<v8::String>("Too many arguments.").ToLocalChecked()));
		return;
	} else if (!args[0]->IsString()) {
		isol->ThrowException(v8::Exception::TypeError(
		    Nan::New<v8::String>("Argument 'uri' must be of type 'String'.").ToLocalChecked()));
		return;
	}

	std::string uri = *v8::String::Utf8Value(args[0]);
	auto        cl  = Controller::GetInstance().connect(uri);
	if (!cl) {
		isol->ThrowException(v8::Exception::Error(Nan::New<v8::String>("Failed to connect.").ToLocalChecked()));
		return;
	}

	return;
}

void js_host(const v8::FunctionCallbackInfo<v8::Value>& args)
{
	auto isol = args.GetIsolate();
	if (args.Length() == 0) {
		isol->ThrowException(
		    v8::Exception::SyntaxError(Nan::New<v8::String>("Too few arguments, usage: host(uri).").ToLocalChecked()));
		return;
	} else if (args.Length() > 1) {
		isol->ThrowException(v8::Exception::SyntaxError(Nan::New<v8::String>("Too many arguments.").ToLocalChecked()));
		return;
	} else if (!args[0]->IsString()) {
		isol->ThrowException(v8::Exception::TypeError(
		    Nan::New<v8::String>("Argument 'uri' must be of type 'String'.").ToLocalChecked()));
		return;
	}

	std::string uri = *v8::String::Utf8Value(args[0]);
	auto        cl  = Controller::GetInstance().host(uri);
	if (!cl) {
		isol->ThrowException(
		    v8::Exception::Error(Nan::New<v8::String>("Failed to host and connect.").ToLocalChecked()));
		return;
	}

	return;
}

void js_disconnect(const v8::FunctionCallbackInfo<v8::Value>& args)
{
	Controller::GetInstance().disconnect();
}

INITIALIZER(js_ipc)
{
	initializerFunctions.push([](v8::Local<v8::Object> exports) {
		// IPC related functions will be under the IPC object.
		auto obj = v8::Object::New(exports->GetIsolate());
		NODE_SET_METHOD(obj, "setServerPath", js_setServerPath);
		NODE_SET_METHOD(obj, "connect", js_connect);
		NODE_SET_METHOD(obj, "host", js_host);
		NODE_SET_METHOD(obj, "disconnect", js_disconnect);
		exports->Set(v8::String::NewFromUtf8(exports->GetIsolate(), "IPC"), obj);
	});
}
