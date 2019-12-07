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

#if defined(_WIN32)
#include "Shlobj.h"
#endif

#include <fstream>
#include <node.h>
#include "fader.hpp"
#include "filter.hpp"
#include "global.hpp"
#include "input.hpp"
#include "isource.hpp"
#include "module.hpp"
#include "nodeobs_api.hpp"
#include "properties.hpp"
#include "scene.hpp"
#include "sceneitem.hpp"
#include "shared.hpp"
#include "transition.hpp"
#include "video.hpp"
#include "volmeter.hpp"
#include "controller.hpp"
#include "nodeobs_autoconfig.hpp"
#include "nodeobs_service.hpp"
#include "nodeobs_settings.hpp"
#include "nodeobs_display.hpp"
#include "callback-manager.hpp"

#include <../obs-studio-server/source/server-nodeobs_api.h>
#include <../obs-studio-server/source/server-nodeobs_autoconfig.h>
#include <../obs-studio-server/source/server-nodeobs_content.h>
//#include <../obs-studio-server/source/server-nodeobs_service.h>
#include <../obs-studio-server/source/server-nodeobs_settings.h>
#include <../obs-studio-server/source/server-nodeobs_display.h>
#include <../obs-studio-server/source/osn-fader.hpp>
#include <../obs-studio-server/source/osn-filter.hpp>
#include <../obs-studio-server/source/osn-global.hpp>
#include <../obs-studio-server/source/osn-input.hpp>
#include <../obs-studio-server/source/osn-module.hpp>
#include <../obs-studio-server/source/osn-properties.hpp>
#include <../obs-studio-server/source/osn-scene.hpp>
#include <../obs-studio-server/source/osn-sceneitem.hpp>
#include <../obs-studio-server/source/osn-source.hpp>
#include <../obs-studio-server/source/osn-transition.hpp>
#include <../obs-studio-server/source/osn-video.hpp>
#include <../obs-studio-server/source/osn-volmeter.hpp>
#include <../obs-studio-server/source/callback-manager.h>

#if defined(_WIN32)
// Checks ForceGPUAsRenderDevice setting
extern "C" __declspec(dllexport) DWORD NvOptimusEnablement = [] {
	LPWSTR       roamingPath;
	std::wstring filePath;
	std::string  line;
	std::fstream file;
	bool         settingValue = true; // Default value (NvOptimusEnablement = 1)

	if (FAILED(SHGetKnownFolderPath(FOLDERID_RoamingAppData, 0, nullptr, &roamingPath))) {
		// Couldn't find roaming app data folder path, assume default value
		return settingValue;
	} else {
		filePath.assign(roamingPath);
		filePath.append(L"\\slobs-client\\basic.ini");
		CoTaskMemFree(roamingPath);
	}

	file.open(filePath);

	if (file.is_open()) {
		while (std::getline(file, line)) {
			if (line.find("ForceGPUAsRenderDevice", 0) != std::string::npos) {
				if (line.substr(line.find('=') + 1) == "false") {
					settingValue = false;
					file.close();
					break;
				}

				break;
			}
		}
	} else {
		//Couldn't open config file, assume default value
		return settingValue;
	}

	// Return setting value
	return settingValue;
}();
#endif

int main(int, char ** , char **){}

// Definition based on addon_register_func, see 'node.h:L384'.
void main_node(v8::Local<v8::Object> exports, v8::Local<v8::Value> module, void* priv)
{
	osn::Global::Register(exports);
	osn::ISource::Register(exports);
	osn::Input::Register(exports);
	osn::Filter::Register(exports);
	osn::Transition::Register(exports);
	osn::Scene::Register(exports);
	osn::SceneItem::Register(exports);
	osn::Properties::Register(exports);
	osn::PropertyObject::Register(exports);
	osn::Fader::Register(exports);
	osn::VolMeter::Register(exports);
	osn::Video::Register(exports);
	osn::Module::Register(exports);

	

	osn::ServerGlobal::Register(client_int.get());
	osn::ServerSource::Register(client_int.get());
	osn::ServerInput::Register(client_int.get());
	osn::ServerFilter::Register(client_int.get());
	osn::ServerTransition::Register(client_int.get());
	osn::ServerScene::Register(client_int.get());
	osn::ServerSceneItem::Register(client_int.get());
	osn::ServerFader::Register(client_int.get());
	osn::ServerVolMeter::Register(client_int.get());
	osn::ServerProperties::Register(client_int.get());
	osn::ServerVideo::Register(client_int.get());
	osn::ServerModule::Register(client_int.get());
	CallbackManagerb::Register(client_int.get());
	OBS_API::Register(client_int.get());
	OBS_content::Register(client_int.get());
	OBS_service::Register(client_int.get());
	OBS_settings::Register(client_int.get());
	ServerAutoConfig::Register(client_int.get());

	//while (initializerFunctions.size() > 0) {
		//initializerFunctions.front()(exports);
		//initializerFunctions.pop();
	//}
	NODE_SET_METHOD(exports, "setServerPath", js_setServerPath);

	NODE_SET_METHOD(exports, "OBS_API_initAPI", api::OBS_API_initAPI);
	NODE_SET_METHOD(exports, "OBS_API_destroyOBS_API", api::OBS_API_destroyOBS_API);
	NODE_SET_METHOD(exports, "OBS_API_getPerformanceStatistics", api::OBS_API_getPerformanceStatistics);
	NODE_SET_METHOD(exports, "SetWorkingDirectory", api::SetWorkingDirectory);
	NODE_SET_METHOD(exports, "StopCrashHandler", api::StopCrashHandler);
	NODE_SET_METHOD(exports, "OBS_API_QueryHotkeys", api::OBS_API_QueryHotkeys);
	NODE_SET_METHOD(exports, "OBS_API_ProcessHotkeyStatus", api::OBS_API_ProcessHotkeyStatus);
	NODE_SET_METHOD(exports, "SetUsername", api::SetUsername);

	NODE_SET_METHOD(exports, "InitializeAutoConfig", autoConfig::InitializeAutoConfig);
	NODE_SET_METHOD(exports, "StartBandwidthTest", autoConfig::StartBandwidthTest);
	NODE_SET_METHOD(exports, "StartStreamEncoderTest", autoConfig::StartStreamEncoderTest);
	NODE_SET_METHOD(exports, "StartRecordingEncoderTest", autoConfig::StartRecordingEncoderTest);
	NODE_SET_METHOD(exports, "StartCheckSettings", autoConfig::StartCheckSettings);
	NODE_SET_METHOD(exports, "StartSetDefaultSettings", autoConfig::StartSetDefaultSettings);
	NODE_SET_METHOD(exports, "StartSaveStreamSettings", autoConfig::StartSaveStreamSettings);
	NODE_SET_METHOD(exports, "StartSaveSettings", autoConfig::StartSaveSettings);
	NODE_SET_METHOD(exports, "TerminateAutoConfig", autoConfig::TerminateAutoConfig);

	NODE_SET_METHOD(exports, "OBS_content_createDisplay", display::OBS_content_createDisplay);
	NODE_SET_METHOD(exports, "OBS_content_destroyDisplay", display::OBS_content_destroyDisplay);
	NODE_SET_METHOD(exports, "OBS_content_getDisplayPreviewOffset", display::OBS_content_getDisplayPreviewOffset);
	NODE_SET_METHOD(exports, "OBS_content_getDisplayPreviewSize", display::OBS_content_getDisplayPreviewSize);
	NODE_SET_METHOD(exports, "OBS_content_createSourcePreviewDisplay", display::OBS_content_createSourcePreviewDisplay);
	NODE_SET_METHOD(exports, "OBS_content_resizeDisplay", display::OBS_content_resizeDisplay);
	NODE_SET_METHOD(exports, "OBS_content_moveDisplay", display::OBS_content_moveDisplay);
	NODE_SET_METHOD(exports, "OBS_content_setPaddingSize", display::OBS_content_setPaddingSize);
	NODE_SET_METHOD(exports, "OBS_content_setPaddingColor", display::OBS_content_setPaddingColor);
	NODE_SET_METHOD(exports, "OBS_content_setShouldDrawUI", display::OBS_content_setShouldDrawUI);
	NODE_SET_METHOD(exports, "OBS_content_setDrawGuideLines", display::OBS_content_setDrawGuideLines);

	NODE_SET_METHOD(exports, "OBS_service_resetAudioContext", service::OBS_service_resetAudioContext);
	NODE_SET_METHOD(exports, "OBS_service_resetVideoContext", service::OBS_service_resetVideoContext);
	NODE_SET_METHOD(exports, "OBS_service_startStreaming", service::OBS_service_startStreaming);
	NODE_SET_METHOD(exports, "OBS_service_startRecording", service::OBS_service_startRecording);
	NODE_SET_METHOD(exports, "OBS_service_startReplayBuffer", service::OBS_service_startReplayBuffer);
	NODE_SET_METHOD(exports, "OBS_service_stopRecording", service::OBS_service_stopRecording);
	NODE_SET_METHOD(exports, "OBS_service_stopStreaming", service::OBS_service_stopStreaming);
	NODE_SET_METHOD(exports, "OBS_service_stopReplayBuffer", service::OBS_service_stopReplayBuffer);
	NODE_SET_METHOD(exports, "OBS_service_connectOutputSignals", service::OBS_service_connectOutputSignals);
	NODE_SET_METHOD(exports, "OBS_service_removeCallback", service::OBS_service_removeCallback);
	NODE_SET_METHOD(exports, "OBS_service_processReplayBufferHotkey", service::OBS_service_processReplayBufferHotkey);
	NODE_SET_METHOD(exports, "OBS_service_getLastReplay", service::OBS_service_getLastReplay);

	NODE_SET_METHOD(exports, "RegisterSourceCallback", RegisterSourceCallback);
	NODE_SET_METHOD(exports, "RemoveSourceCallback", RemoveSourceCallback);

	NODE_SET_METHOD(exports, "OBS_settings_getSettings", settings::OBS_settings_getSettings);
	NODE_SET_METHOD(exports, "OBS_settings_saveSettings", settings::OBS_settings_saveSettings);
	NODE_SET_METHOD(exports, "OBS_settings_getListCategories", settings::OBS_settings_getListCategories);
};

NODE_MODULE(obs_studio_node, main_node); // Upgrade to NAPI_MODULE once N-API hits stable/beta.
