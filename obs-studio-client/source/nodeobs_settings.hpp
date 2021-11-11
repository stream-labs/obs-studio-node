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

#include <napi.h>

namespace settings
{
	struct DeviceInfo
	{
		std::string description;
		std::string id;
	};

	void Init(Napi::Env env, Napi::Object exports);

	Napi::Value OBS_settings_getSettings(const Napi::CallbackInfo& info);
	void OBS_settings_saveSettings(const Napi::CallbackInfo& info);
	Napi::Value OBS_settings_getListCategories(const Napi::CallbackInfo& info);

	Napi::Value OBS_settings_getInputAudioDevices(const Napi::CallbackInfo& info);
	Napi::Value OBS_settings_getOutputAudioDevices(const Napi::CallbackInfo& info);
	Napi::Value OBS_settings_getVideoDevices(const Napi::CallbackInfo& info);


	static std::vector<std::string> getListCategories(void);
}
