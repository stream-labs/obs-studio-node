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

#include "shared.hpp"
#include <iostream>
#include <fstream>
#include <varargs.h>
#include <mutex>

#ifdef _WIN32
#include <windows.h>
#define SYSERROR()  GetLastError()
#else
#include <errno.h>
#define SYSERROR()  errno
#endif

std::queue<std::function<void(v8::Local<v8::Object>)>> initializerFunctions;

void replaceAll(std::string& str, const std::string& from, const std::string& to) {
	if (from.empty())
		return;
	size_t start_pos = 0;
	while ((start_pos = str.find(from, start_pos)) != std::string::npos) {
		str.replace(start_pos, from.length(), to);
		start_pos += to.length(); // In case 'to' contains 'from', like replacing 'x' with 'yx'
	}
};

#pragma region Logging
static bool global_log_stdout_enabled = true;
static bool global_log_stderr_enabled = false;
static bool global_log_debug_enabled = true;
static bool global_log_file_enabled = false;
static std::string global_log_file_path = "";
static std::ofstream global_log_file_stream;
std::chrono::high_resolution_clock hrc;
static std::chrono::high_resolution_clock::time_point global_log_start = std::chrono::high_resolution_clock::now();

void shared::log_stdout_enable(bool enabled) {
	global_log_stdout_enabled = enabled;
}

void shared::log_stderr_enable(bool enabled) {
	global_log_stderr_enabled = enabled;
}

void shared::log_debug_enable(bool enabled) {
	global_log_debug_enabled = enabled;
}

void shared::log_file_enable(bool enabled) {
	global_log_file_enabled = enabled;
}

void shared::log_file_path(std::string path) {
	if (path != global_log_file_path) {
		if (global_log_file_stream.good()) {
			global_log_file_stream.close();
		}
		global_log_file_stream.open(path, std::ios::out);
		if (!global_log_file_stream.is_open()) {
			uint32_t error = SYSERROR();
		}
	}
	global_log_file_path = path;
}

void shared::log(std::string format, ...) {
	std::vector<char> message_buffer;
	std::vector<char> timestamp_buffer;

	// Generate Timestamp
	auto timeSinceStart = (std::chrono::high_resolution_clock::now() - global_log_start);
	auto hours = std::chrono::duration_cast<std::chrono::hours>(timeSinceStart);
	timeSinceStart -= hours;
	auto minutes = std::chrono::duration_cast<std::chrono::minutes>(timeSinceStart);
	timeSinceStart -= minutes;
	auto seconds = std::chrono::duration_cast<std::chrono::seconds>(timeSinceStart);
	timeSinceStart -= seconds;
	auto milliseconds = std::chrono::duration_cast<std::chrono::milliseconds>(timeSinceStart);
	timeSinceStart -= milliseconds;
	auto microseconds = std::chrono::duration_cast<std::chrono::microseconds>(timeSinceStart);
	timeSinceStart -= microseconds;
	auto nanoseconds = std::chrono::duration_cast<std::chrono::nanoseconds>(timeSinceStart);

	const char* timestamp_format = "%.2d:%.2d:%.2d.%.3d.%.3d.%.3d";
#define timestamp_args hours.count(), minutes.count(), seconds.count(),	milliseconds.count(), microseconds.count(), nanoseconds.count()
	timestamp_buffer.resize(_scprintf(timestamp_format, timestamp_args) + 1);
	sprintf_s(timestamp_buffer.data(), timestamp_buffer.size(), timestamp_format, timestamp_args);
#undef timestamp_args

	// Generate Message
	va_list args;
	va_start(args, format);
	message_buffer.resize(_vscprintf(format.c_str(), args) + 1);
	vsprintf_s(message_buffer.data(), message_buffer.size(), format.c_str(), args);
	va_end(args);

	// Log each line individually
	size_t message_begin = 0;
	size_t message_end = 0;
	std::string message;
	const char* final_format = "[%*s] %*s\n";
	std::vector<char> final_buffer;
	for (size_t p = 0, end = message_buffer.size(); p < end; p++) {
		char c = message_buffer[p];
		if (p < (end - 1) && (c == '\r' && (message_buffer[p + 1] == '\n'))) {
			message_end = p;
			message = std::string(message_buffer.data() + message_begin, message_buffer.data() + message_end);
			message_begin = p + 2;
		} else if (c == '\n' || c == '\r') {
			message_end = p;
			message = std::string(message_buffer.data() + message_begin, message_buffer.data() + message_end);
			message_begin = p + 1;
		} else if (p == end - 1) {
			message_end = end;
			message = std::string(message_buffer.data() + message_begin, message_buffer.data() + message_end);
		}

		if (message_end != 0) {
			final_buffer.resize(_scprintf(final_format,
				timestamp_buffer.size(), timestamp_buffer.data(),
				message.length(), message.data()) + 1);
			sprintf_s(final_buffer.data(), final_buffer.size(), final_format,
				timestamp_buffer.size(), timestamp_buffer.data(),
				message.length(), message.data());
			final_buffer[final_buffer.size() - 1] = 0;

			if (global_log_stdout_enabled) {
				fwrite(final_buffer.data(), sizeof(char), final_buffer.size(), stdout);
			}
			if (global_log_stderr_enabled) {
				fwrite(final_buffer.data(), sizeof(char), final_buffer.size(), stderr);
			}
			if (global_log_debug_enabled) {
			#ifdef _WIN32
				if (IsDebuggerPresent()) {
					int wNum = MultiByteToWideChar(CP_UTF8, 0, final_buffer.data(), -1, NULL, 0);
					if (wNum > 1) {
						std::wstring wide_buf;
						std::mutex wide_mutex;

						std::lock_guard<std::mutex> lock(wide_mutex);
						wide_buf.reserve(wNum + 1);
						wide_buf.resize(wNum - 1);
						MultiByteToWideChar(CP_UTF8, 0, final_buffer.data(), -1, &wide_buf[0],
							wNum);

						OutputDebugStringW(wide_buf.c_str());
					}
				}
			#endif
			}
			if (global_log_file_enabled) {
				if (global_log_file_stream.is_open()) {
					global_log_file_stream << final_buffer.data();
				}
			}

			message_end = 0;
		}
	}
}
#pragma endregion Logging

shared::LogWarnTimer::LogWarnTimer(std::string name, std::chrono::nanoseconds const warn_limit) {
	this->name = name;
	this->warn_limit = warn_limit;
	this->begin = std::chrono::high_resolution_clock::now();
}

shared::LogWarnTimer::~LogWarnTimer() {
	auto end = std::chrono::high_resolution_clock::now();
	std::chrono::nanoseconds dur =
		std::chrono::duration_cast<std::chrono::nanoseconds>(end - begin);

	if (dur >= warn_limit) {
		shared::log("%s: Took over %llu nanoseconds (limit was %llu nanoseconds).",
			name.c_str(), dur.count(), warn_limit.count());
	}
}
