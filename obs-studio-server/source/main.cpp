// Server program for the OBS Studio node module.
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

#include <iostream>
#include <thread>
#include <chrono>
#include <vector>
#include <inttypes.h>
#include <memory>
#include <ipc-function.hpp>
#include <ipc-class.hpp>
#include <ipc-server.hpp>
#include "osn-global.hpp"
#include "osn-source.hpp"
#include "osn-input.hpp"
#include "osn-filter.hpp"
#include "osn-transition.hpp"
#include "osn-scene.hpp"
#include "osn-sceneitem.hpp"
#include "osn-properties.hpp"
#include "nodeobs_api.h"
#include "nodeobs_content.h"
#include "nodeobs_service.h"
#include "nodeobs_settings.h"
#include "error.hpp"
#include <chrono>
#include "osn-fader.hpp"
#include "osn-volmeter.hpp"

struct ServerData {
	std::mutex mtx;
	std::chrono::high_resolution_clock::time_point last_connect, last_disconnect;
	size_t count_connected = 0;
};

bool ServerConnectHandler(void* data, os::ClientId_t) {
	ServerData* sd = reinterpret_cast<ServerData*>(data);
	std::unique_lock<std::mutex> ulock(sd->mtx);
	sd->last_connect = std::chrono::high_resolution_clock::now();
	sd->count_connected++;
	return true;
}

void ServerDisconnectHandler(void* data, os::ClientId_t) {
	ServerData* sd = reinterpret_cast<ServerData*>(data);
	std::unique_lock<std::mutex> ulock(sd->mtx);
	sd->last_disconnect = std::chrono::high_resolution_clock::now();
	sd->count_connected--;
}

namespace System {
	static void Shutdown(void* data, const int64_t id, const std::vector<ipc::value>& args, std::vector<ipc::value>& rval) {
		bool* shutdown = (bool*)data;
		*shutdown = true;
		rval.push_back(ipc::value((uint64_t)ErrorCode::Ok));
		return;
	}
}

int main(int argc, char* argv[]) {
	// Usage:
	// argv[0] = Path to this application. (Usually given by default if run via path-based command!)
	// argv[1] = Path to a named socket.

	if (argc != 2) {
		std::cerr << "There must be exactly one parameter." << std::endl;
		return -1;
	}

	// Instance
	ipc::server myServer;
	bool doShutdown = false;
	ServerData sd;
	sd.last_disconnect = sd.last_connect = std::chrono::high_resolution_clock::now();
	sd.count_connected = 0;

	// Initialize Singleton Source Storage
	osn::Source::Initialize();

	// Classes
	/// System
	{
		std::shared_ptr<ipc::collection> cls = std::make_shared<ipc::collection>("System");
		cls->register_function(std::make_shared<ipc::function>("Shutdown", std::vector<ipc::type>{}, System::Shutdown, &doShutdown));
		myServer.register_collection(cls);
	};

	/// OBS Studio Node
	osn::Global::Register(myServer);
	osn::Source::Register(myServer);
	osn::Input::Register(myServer);
	osn::Filter::Register(myServer);
	osn::Transition::Register(myServer);
	osn::Scene::Register(myServer);
	osn::SceneItem::Register(myServer);
	osn::Fader::Register(myServer);
	osn::VolMeter::Register(myServer);
	osn::Properties::Register(myServer);
	OBS_API::Register(myServer);
	OBS_content::Register(myServer);
	OBS_service::Register(myServer);
	OBS_settings::Register(myServer);

	// Register Connect/Disconnect Handlers
	myServer.set_connect_handler(ServerConnectHandler, &sd);
	myServer.set_disconnect_handler(ServerDisconnectHandler, &sd);

	// Initialize Server
	try {
		myServer.initialize(argv[1]);
	} catch (...) {
		std::cerr << "Failed to initialize server" << std::endl;
		return -2;
	}

	while (!doShutdown) {
		if (sd.count_connected == 0) {
			auto tp = std::chrono::high_resolution_clock::now();
			auto delta = tp - sd.last_disconnect;
			if (std::chrono::duration_cast<std::chrono::milliseconds>(delta).count() > 1000) {
				doShutdown = true;
			}
		}
		std::this_thread::sleep_for(std::chrono::milliseconds(10));
	}

	// Finalize Singleton Source Storage
	osn::Source::Finalize();

	// Finalize Server
	myServer.finalize();

	return 0;
}
