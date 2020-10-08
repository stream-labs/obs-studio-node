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
#include <mutex>
#include "utility-v8.hpp"


struct Permissions
{
    bool webcam;
    bool mic;
};

namespace api
{
    void Init(Napi::Env env, Napi::Object exports);

	Napi::Value OBS_API_initAPI(const Napi::CallbackInfo& info);
	Napi::Value OBS_API_destroyOBS_API(const Napi::CallbackInfo& info);
	Napi::Value OBS_API_getPerformanceStatistics(const Napi::CallbackInfo& info);
	Napi::Value SetWorkingDirectory(const Napi::CallbackInfo& info);
	Napi::Value InitShutdownSequence(const Napi::CallbackInfo& info);
	Napi::Value OBS_API_QueryHotkeys(const Napi::CallbackInfo& info);
	Napi::Value OBS_API_ProcessHotkeyStatus(const Napi::CallbackInfo& info);
	Napi::Value SetUsername(const Napi::CallbackInfo& info);
	Napi::Value GetPermissionsStatus(const Napi::CallbackInfo& info);
	Napi::Value RequestPermissions(const Napi::CallbackInfo& info);
}
