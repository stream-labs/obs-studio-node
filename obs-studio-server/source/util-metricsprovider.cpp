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

#include "util-metricsprovider.h"

#include <chrono>
#include <iostream>
#include <string>
#include <thread>
#include <vector>

#if defined(_WIN32)

#include <io.h>
#include <windows.h>
#include "TCHAR.h"

#endif

///////////////////////
// MetricsPipeClient //
///////////////////////
#ifdef WIN32
util::MetricsProvider::~MetricsProvider()
{
	m_StopPolling = true;
	if (m_PollingThread.joinable()) {
		m_PollingThread.join();
	}

	MetricsMessage message;

	// If we should blame the server
	if (m_BlameServer) {
		message.type = MessageType::Status;
		strcpy_s(message.param1, "Backend Crash");
	} else if (m_BlameFrontend) {
		message.type = MessageType::Status;
		strcpy_s(message.param1, "Frontend Crash");
	} else if (m_BlameUser) {
		message.type = MessageType::Status;
		strcpy_s(message.param1, "User Crash");
	} else {
		message.type = MessageType::Shutdown;
	}

	if (m_PipeIsOpen) {
		SendPipeMessage(message);
		CloseHandle(m_Pipe);
	}
}

bool util::MetricsProvider::Initialize(const std::string &pipe_name, std::string current_version, bool send_messages_async)
{
	m_Pipe = CreateFileA(pipe_name.c_str(), GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);

	if (m_Pipe == NULL || m_Pipe == INVALID_HANDLE_VALUE) {
		std::cout << "Failed to create outbound pipe instance." << std::endl;
		// look up error code here using GetLastError()
		m_PipeIsOpen = false;
		return false;
	}

	m_PipeIsOpen = true;

	// Send the pid
	MetricsMessage message;
	DWORD pid = GetCurrentProcessId();
	message.type = MessageType::Pid;
	memcpy(message.param1, &pid, sizeof(DWORD));
	bool result = SendPipeMessage(message);

	// Start pooling and send our tag
	StartPolling(send_messages_async);
	SendTag("version", current_version);

	return true;
}

void util::MetricsProvider::StartPolling(bool send_messages_async)
{
	// Update how we should send the messages, it it's synchronous just exist
	m_SendMessagesAsync = send_messages_async;
	if (!m_SendMessagesAsync) {
		return;
	}

	m_PollingThread = std::thread([=]() {
		while (!m_StopPolling) {
			// Check if we have data to be sent
			{
				m_PollingMutex.lock();
				std::queue<MetricsMessage> pendingData = std::move(m_AsyncData);
				m_PollingMutex.unlock();

				while (!pendingData.empty()) {
					auto message = pendingData.front();
					pendingData.pop();

					bool result = SendPipeMessage(message);
					if (!result) {
						// ?
					}
				}
			}

			std::this_thread::sleep_for(std::chrono::milliseconds(50));
		}
	});
}

void util::MetricsProvider::SendStatus(const std::string &status)
{
	MetricsMessage message = {};
	message.type = MessageType::Status;
	strcpy_s(message.param1, status.c_str());

	PrepareMessage(message);
}

void util::MetricsProvider::SendTag(const std::string &tag, const std::string &value)
{
	MetricsMessage message = {};
	message.type = MessageType::Tag;
	strcpy_s(message.param1, tag.c_str());
	strcpy_s(message.param2, value.c_str());

	PrepareMessage(message);
}

void util::MetricsProvider::PrepareMessage(MetricsMessage &message)
{
	std::lock_guard<std::mutex> l(m_PollingMutex);

	// Check if the message should be send asynchronous, else send it directly
	if (m_SendMessagesAsync) {
		m_AsyncData.emplace(message);
	} else {
		SendPipeMessage(message);
	}
}

bool util::MetricsProvider::SendPipeMessage(MetricsMessage &message)
{
	if (m_PipeIsOpen) {
		DWORD numBytesWritten = 0;
		return WriteFile(m_Pipe,                 // handle to our outbound pipe
				 &message,               // data to send
				 sizeof(MetricsMessage), // length of data to send (bytes)
				 &numBytesWritten,       // will store actual amount of data sent
				 NULL                    // not using overlapped IO
		);
	}

	return false;
}

void util::MetricsProvider::BlameServer()
{
	m_BlameServer = true;

	MetricsMessage message = {};
	message.type = MessageType::Blame;
	strcpy_s(message.param1, "Backend Crash");

	PrepareMessage(message);
}

void util::MetricsProvider::BlameUser()
{
	m_BlameUser = true;

	MetricsMessage message = {};
	message.type = MessageType::Blame;
	strcpy_s(message.param1, "User Crash");

	PrepareMessage(message);
}

void util::MetricsProvider::BlameFrontend()
{
	m_BlameFrontend = true;

	MetricsMessage message = {};
	message.type = MessageType::Blame;
	strcpy_s(message.param1, "Frontend Crash");

	PrepareMessage(message);
}
#endif
