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

#if defined(_WIN32)
// Checks DisableGPUAsRenderDevice setting
extern "C" __declspec(dllexport) DWORD NvOptimusEnablement = [] {
	LPWSTR       roamingPath;
	std::wstring filePath;
	std::string  line;
	std::fstream file;
	bool         settingValue = false;

	if (FAILED(SHGetKnownFolderPath(FOLDERID_RoamingAppData, 0, nullptr, &roamingPath))) {
		// Wasn't able to find roaming app data path
		return 1;
	} else {
		filePath.assign(roamingPath);
		filePath.append(L"\\slobs-client\\basic.ini");
		CoTaskMemFree(roamingPath);
	}

	file.open(filePath);

	if (file.is_open()) {
		while (std::getline(file, line)) {
			if (line.find("DisableGPUAsRenderDevice", 0) != std::string::npos) {
				if (line.substr(line.find('=') + 1) == "true") {
					settingValue = true;
					file.close();
					break;
				}
			}
		}
	} else {
		// Wasn't able to open config file
		return 1;
	}

	if (settingValue) {
		// Disable high performance graphics rendering
		return 0;
	}

	// Enable high performance graphics rendering
	return 1;
}();
#endif

// Definition based on addon_register_func, see 'node.h:L384'.
void main(v8::Local<v8::Object> exports, v8::Local<v8::Value> module, void* priv)
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

	while (initializerFunctions.size() > 0) {
		initializerFunctions.front()(exports);
		initializerFunctions.pop();
	}
};

NODE_MODULE(obs_studio_node, main); // Upgrade to NAPI_MODULE once N-API hits stable/beta.
