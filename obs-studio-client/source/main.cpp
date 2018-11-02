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

#include <node.h>
#include "fader.hpp"
#include "filter.hpp"
#include "global.hpp"
#include "input.hpp"
#include "isource.hpp"
#include "nodeobs_api.hpp"
#include "properties.hpp"
#include "scene.hpp"
#include "sceneitem.hpp"
#include "shared.hpp"
#include "transition.hpp"
#include "video.hpp"
#include "volmeter.hpp"
#include "module.hpp"

extern "C" __declspec(dllexport) DWORD NvOptimusEnablement = 1;

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
