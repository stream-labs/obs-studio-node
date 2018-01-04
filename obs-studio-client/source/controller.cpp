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
#include <string>
#include <sstream>

#ifdef _WIN32
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>
#endif

Controller::Controller() {

}

Controller::~Controller() {
	Disconnect();
}

std::shared_ptr<IPC::Client> Controller::Host(std::string uri) {
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
	std::shared_ptr<IPC::Client> cl;
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

std::shared_ptr<IPC::Client> Controller::Connect(std::string uri) {
	if (m_isServer)
		return nullptr;

	// Try and connect.
	std::shared_ptr<IPC::Client> cl;
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

std::shared_ptr<IPC::Client> Controller::GetConnection() {
	return m_connection;
}
