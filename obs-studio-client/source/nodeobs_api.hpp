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

#include <iostream>
#include <nan.h>
#include <node.h>

namespace api
{
	static void OBS_API_initAPI(const v8::FunctionCallbackInfo<v8::Value>& args);
	static void OBS_API_destroyOBS_API(const v8::FunctionCallbackInfo<v8::Value>& args);
	static void OBS_API_getPerformanceStatistics(const v8::FunctionCallbackInfo<v8::Value>& args);
	static void SetWorkingDirectory(const v8::FunctionCallbackInfo<v8::Value>& args);
	static void StopCrashHandler(const v8::FunctionCallbackInfo<v8::Value>& args);
	static void OBS_API_QueryHotkeys(const v8::FunctionCallbackInfo<v8::Value>& args);
	static void OBS_API_ProcessHotkeyStatus(const v8::FunctionCallbackInfo<v8::Value>& args);
	static void LogMessage(const v8::FunctionCallbackInfo<v8::Value>& args);
} // namespace api
