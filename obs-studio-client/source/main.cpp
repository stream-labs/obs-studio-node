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
NAN_MODULE_INIT(main_node)
{
	osn::Global::Register(target);
	osn::ISource::Register(target);
	osn::Input::Register(target);
	osn::Filter::Register(target);
	osn::Transition::Register(target);
	osn::Scene::Register(target);
	osn::SceneItem::Register(target);
	osn::Properties::Register(target);
	osn::PropertyObject::Register(target);
	osn::Fader::Register(target);
	osn::VolMeter::Register(target);
	osn::Video::Register(target);
	osn::Module::Register(target);

	while (initializerFunctions.size() > 0) {
		initializerFunctions.front()(target);
		initializerFunctions.pop();
	}
};

NAN_MODULE_WORKER_ENABLED(obs_studio_node, main_node); // Upgrade to NAPI_MODULE once N-API hits stable/beta.
