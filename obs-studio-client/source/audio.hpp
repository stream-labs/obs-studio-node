/******************************************************************************
    Copyright (C) 2016-2022 by Streamlabs (General Workings Inc)

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
#include <napi.h>

namespace osn {
class Audio : public Napi::ObjectWrap<osn::Audio> {
public:
	static Napi::FunctionReference constructor;
	static Napi::Object Init(Napi::Env env, Napi::Object exports);
	Audio(const Napi::CallbackInfo &info);

	static Napi::Value GetAudioContext(const Napi::CallbackInfo &info);
	static void SetAudioContext(const Napi::CallbackInfo &info, const Napi::Value &value);
	static Napi::Value GetLegacySettings(const Napi::CallbackInfo &info);
	static void SetLegacySettings(const Napi::CallbackInfo &info, const Napi::Value &value);

	static Napi::Value GetMonitoringDevice(const Napi::CallbackInfo &info);
	static void SetMonitoringDevice(const Napi::CallbackInfo &info, const Napi::Value &value);
	static Napi::Value GetMonitoringDeviceLegacy(const Napi::CallbackInfo &info);
	static Napi::Value GetMonitoringDevices(const Napi::CallbackInfo &info);
	static Napi::Value GetDisableAudioDucking(const Napi::CallbackInfo &info);
	static void SetDisableAudioDucking(const Napi::CallbackInfo &info, const Napi::Value &value);
	static Napi::Value GetDisableAudioDuckingLegacy(const Napi::CallbackInfo &info);
};
}