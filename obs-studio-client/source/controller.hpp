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

#pragma once
#include <memory>
#include <map>
#include <string>
#include "ipc-client.hpp"
#include <napi.h>

#ifdef _WIN32
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>
#else
#define DWORD unsigned long
#endif

struct ProcessInfo
{
	uint64_t handle;
	uint64_t id;
	DWORD    exit_code;

	enum ExitCode
	{
		STILL_RUNNING = 259,
		VERSION_MISMATCH = 252,
		OTHER_ERROR = 253,
		NORMAL_EXIT = 0
	};

	typedef std::map<ProcessInfo::ExitCode, std::string> ProcessDescriptionMap;

	private:
		static ProcessDescriptionMap descriptions;
		static ProcessDescriptionMap initDescriptions();

	public:
		ProcessInfo() : handle(0), id(0), exit_code(0)
		{
	
		};
		ProcessInfo(uint64_t h, uint64_t i) : handle(h), id(i), exit_code(0)
		{
		
		}
		static std::string getDescription(DWORD key) {
		    ProcessInfo::ExitCode k = static_cast<ProcessInfo::ExitCode>(key);
		    if (descriptions.find(k) != descriptions.end()) {
			    return descriptions[k];
			}
		    return "Generic Error";
		}
};

class Controller
{
	public:
	static Controller& GetInstance()
	{
		static Controller _inst;
		return _inst;
	}

	private:
	Controller();
	~Controller();

	public: // C++11
	Controller(Controller const&) = delete;
	void operator=(Controller const&) = delete;

	static void Init(Napi::Env env, Napi::Object exports);

	public:
	std::shared_ptr<ipc::client> host(const std::string& uri);

	std::shared_ptr<ipc::client> connect(const std::string& uri);

	DWORD GetExitCode();

	void disconnect();

	std::shared_ptr<ipc::client> GetConnection();

	private:
	bool                         m_isServer = false;
	std::shared_ptr<ipc::client> m_connection;
	ProcessInfo                  procId;
};
