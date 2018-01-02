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
#include "obs-main.hpp"

namespace System {
	IPC::Value Shutdown(int64_t id, void* data, std::vector<IPC::Value>) {
		bool* shutdown = (bool*)data;
		*shutdown = true;
		return IPC::Value(1);
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
	IPC::Server myServer;
	bool doShutdown = false;

	// Classes
	OBS::Main::Register(myServer);
	/// System
	{
		IPC::Class system("System");
		system.RegisterFunction(std::make_shared<IPC::Function>("Shutdown", System::Shutdown, &doShutdown));
		myServer.RegisterClass(system);
	};

	try {
		myServer.Initialize(argv[1]);
	} catch (...) {
		std::cerr << "Failed to initialize server" << std::endl;
		return -2;
	}

	while (!doShutdown) {
		std::this_thread::sleep_for(std::chrono::milliseconds(10));
	}

	myServer.Finalize();
	return 0;
}
