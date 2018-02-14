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

#pragma once
#include "osn-source.hpp"
#include "osn-scene.hpp"
#include <list>
#include <memory>

namespace shared {
	//// An object used by the server to determine which objects are in use by the client connected.
	//
	class ClientStorage {
		std::list<std::shared_ptr<osn::Source>> activeSources;
		//std::list<std::shared_ptr<osn::Scene>> activeScenes;
		//std::list<std::shared_ptr<osn::SceneItem>> activeScenes;
		//std::list<std::shared_ptr<osn::Transition>> activeScenes;
		//std::list<std::shared_ptr<osn::Video>> activeScenes;
		//std::list<std::shared_ptr<osn::Filter>> activeScenes;
		//std::list<std::shared_ptr<osn::Input>> activeScenes;

		public:
		ClientStorage();
		~ClientStorage();


	};


}