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

#include "osn-scene.hpp"
#include "error.hpp"
#include <list>

// Scenes are a little weird, they need SceneItems which are virtual objects that don't exist for other Scenes.
// May have to figure something out there...

void osn::Scene::Register(ipc::server& srv) {
	std::shared_ptr<ipc::collection> cls = std::make_shared<ipc::collection>("Scene");
	cls->register_function(std::make_shared<ipc::function>("Create", std::vector<ipc::type>{ipc::type::String}, Create));
	cls->register_function(std::make_shared<ipc::function>("CreatePrivate", std::vector<ipc::type>{ipc::type::String}, CreatePrivate));
	cls->register_function(std::make_shared<ipc::function>("FromName", std::vector<ipc::type>{ipc::type::String}, FromName));

	cls->register_function(std::make_shared<ipc::function>("AsSource", std::vector<ipc::type>{ipc::type::UInt64}, AsSource));
	cls->register_function(std::make_shared<ipc::function>("Duplicate", std::vector<ipc::type>{ipc::type::UInt64, ipc::type::String, ipc::type::Int32}, Duplicate));

	cls->register_function(std::make_shared<ipc::function>("AddSource", std::vector<ipc::type>{ipc::type::UInt64, ipc::type::UInt64}, AddSource));
	cls->register_function(std::make_shared<ipc::function>("FindItem", std::vector<ipc::type>{ipc::type::UInt64, ipc::type::String}, FindItem));
	cls->register_function(std::make_shared<ipc::function>("MoveItem", std::vector<ipc::type>{ipc::type::UInt64, ipc::type::UInt64, ipc::type::Int32}, MoveItem));
	cls->register_function(std::make_shared<ipc::function>("GetItem", std::vector<ipc::type>{ipc::type::UInt64, ipc::type::UInt64}, GetItem));
	cls->register_function(std::make_shared<ipc::function>("GetItems", std::vector<ipc::type>{ipc::type::UInt64}, GetItems));
	cls->register_function(std::make_shared<ipc::function>("GetItemsInRange", std::vector<ipc::type>{ipc::type::UInt64, ipc::type::UInt64, ipc::type::UInt64}, GetItemsInRange));

	cls->register_function(std::make_shared<ipc::function>("Connect", std::vector<ipc::type>{ipc::type::UInt64}, Connect));
	cls->register_function(std::make_shared<ipc::function>("Disconnect", std::vector<ipc::type>{ipc::type::UInt64}, Disconnect));
	srv.register_collection(cls);
}

void osn::Scene::Create(void* data, const int64_t id, const std::vector<ipc::value>& args, std::vector<ipc::value>& rval) {
	obs_scene_t* scene = obs_scene_create(args[0].value_str.c_str());
	if (!scene) {
		rval.push_back(ipc::value((uint64_t)ErrorCode::Error));
		rval.push_back(ipc::value("Failed to create scene."));
		return;
	}

	obs_source_t* source = obs_scene_get_source(scene);
	if (!source) {
		rval.push_back(ipc::value((uint64_t)ErrorCode::Error));
		rval.push_back(ipc::value("Failed to get source from scene."));
		return;
	}

	uint64_t uid = osn::Source::GetInstance()->Allocate(source);
	if (uid == UINT64_MAX) {
		rval.push_back(ipc::value((uint64_t)ErrorCode::OutOfIndexes));
		rval.push_back(ipc::value("Index list is full."));
		return;
	}

	rval.push_back(ipc::value((uint64_t)ErrorCode::Ok));
	rval.push_back(ipc::value(uid));
	return;
}

void osn::Scene::CreatePrivate(void* data, const int64_t id, const std::vector<ipc::value>& args, std::vector<ipc::value>& rval) {
	obs_scene_t* scene = obs_scene_create_private(args[0].value_str.c_str());
	if (!scene) {
		rval.push_back(ipc::value((uint64_t)ErrorCode::Error));
		rval.push_back(ipc::value("Failed to create scene."));
		return;
	}

	obs_source_t* source = obs_scene_get_source(scene);
	if (!source) {
		rval.push_back(ipc::value((uint64_t)ErrorCode::Error));
		rval.push_back(ipc::value("Failed to get source from scene."));
		return;
	}

	uint64_t uid = osn::Source::GetInstance()->Allocate(source);
	if (uid == UINT64_MAX) {
		rval.push_back(ipc::value((uint64_t)ErrorCode::OutOfIndexes));
		rval.push_back(ipc::value("Index list is full."));
		return;
	}

	rval.push_back(ipc::value((uint64_t)ErrorCode::Ok));
	rval.push_back(ipc::value(uid));
	return;
}

void osn::Scene::FromName(void* data, const int64_t id, const std::vector<ipc::value>& args, std::vector<ipc::value>& rval) {
	obs_source_t* source = obs_get_source_by_name(args[0].value_str.c_str());
	if (!source) {
		rval.push_back(ipc::value((uint64_t)ErrorCode::Error));
		rval.push_back(ipc::value("Failed to get source from scene."));
		return;
	}

	uint64_t uid = osn::Source::GetInstance()->Get(source);
	if (uid == UINT64_MAX) {
		// This is an impossible case, but we handle it in case it happens.
		obs_source_release(source);
	#ifdef DEBUG // Debug should throw an error for debuggers to catch.
		throw std::runtime_error("Source found but not indexed.");
	#endif
		rval.push_back(ipc::value((uint64_t)ErrorCode::CriticalError));
		rval.push_back(ipc::value("Source found but not indexed."));
		return;
	}

	obs_source_release(source);

	rval.push_back(ipc::value((uint64_t)ErrorCode::Ok));
	rval.push_back(ipc::value(uid));
	return;
}

void osn::Scene::AsSource(void* data, const int64_t id, const std::vector<ipc::value>& args, std::vector<ipc::value>& rval) {
	// Scenes are stored as such.
	rval.push_back(ipc::value((uint64_t)ErrorCode::Ok));
	rval.push_back(ipc::value(args[0].value_union.ui64));
	return;
}

void osn::Scene::Duplicate(void* data, const int64_t id, const std::vector<ipc::value>& args, std::vector<ipc::value>& rval) {
	obs_source_t* source = osn::Source::GetInstance()->Get(args[0].value_union.ui64);
	if (!source) {
		rval.push_back(ipc::value((uint64_t)ErrorCode::InvalidReference));
		rval.push_back(ipc::value("Source reference is not valid."));
		return;
	}

	obs_scene_t* scene = obs_scene_from_source(source);
	if (!scene) {
		rval.push_back(ipc::value((uint64_t)ErrorCode::InvalidReference));
		rval.push_back(ipc::value("Source reference is not a scene."));
		return;
	}

	obs_scene_t* scene2 = obs_scene_duplicate(scene, args[1].value_str.c_str(), (obs_scene_duplicate_type)args[2].value_union.i32);
	if (!scene2) {
		rval.push_back(ipc::value((uint64_t)ErrorCode::Error));
		rval.push_back(ipc::value("Failed to duplicate scene."));
		return;
	}

	obs_source_t* source2 = obs_scene_get_source(scene2);
	if (!source2) {
		rval.push_back(ipc::value((uint64_t)ErrorCode::Error));
		rval.push_back(ipc::value("Failed to get source from duplicate scene."));
		return;
	}

	uint64_t uid = osn::Source::GetInstance()->Allocate(source2);
	if (uid == UINT64_MAX) {
		rval.push_back(ipc::value((uint64_t)ErrorCode::OutOfIndexes));
		rval.push_back(ipc::value("Index list is full."));
		return;
	}

	rval.push_back(ipc::value((uint64_t)ErrorCode::Ok));
	rval.push_back(ipc::value(uid));
	return;
}

void osn::Scene::AddSource(void* data, const int64_t id, const std::vector<ipc::value>& args, std::vector<ipc::value>& rval) {
	obs_source_t* source = osn::Source::GetInstance()->Get(args[0].value_union.ui64);
	if (!source) {
		rval.push_back(ipc::value((uint64_t)ErrorCode::InvalidReference));
		rval.push_back(ipc::value("Source reference is not valid."));
		return;
	}

	obs_scene_t* scene = obs_scene_from_source(source);
	if (!scene) {
		rval.push_back(ipc::value((uint64_t)ErrorCode::InvalidReference));
		rval.push_back(ipc::value("Source reference is not a scene."));
		return;
	}

	obs_source_t* added_source = osn::Source::GetInstance()->Get(args[1].value_union.ui64);
	if (!added_source) {
		rval.push_back(ipc::value((uint64_t)ErrorCode::InvalidReference));
		rval.push_back(ipc::value("Source reference to add is not valid."));
		return;
	}

	obs_sceneitem_t* item = obs_scene_add(scene, added_source);

	rval.push_back(ipc::value((uint64_t)ErrorCode::Ok));
	rval.push_back(ipc::value(args[0].value_union.ui64));
	rval.push_back(ipc::value((uint64_t)reinterpret_cast<intptr_t>(item)));
	return;
}

void osn::Scene::FindItem(void* data, const int64_t id, const std::vector<ipc::value>& args, std::vector<ipc::value>& rval) {
	obs_source_t* source = osn::Source::GetInstance()->Get(args[0].value_union.ui64);
	if (!source) {
		rval.push_back(ipc::value((uint64_t)ErrorCode::InvalidReference));
		rval.push_back(ipc::value("Source reference is not valid."));
		return;
	}

	obs_scene_t* scene = obs_scene_from_source(source);
	if (!scene) {
		rval.push_back(ipc::value((uint64_t)ErrorCode::InvalidReference));
		rval.push_back(ipc::value("Source reference is not a scene."));
		return;
	}

	obs_sceneitem_t* item = obs_scene_find_source(scene, args[1].value_str.c_str());
	if (!item) {
		rval.push_back(ipc::value((uint64_t)ErrorCode::Error));
		rval.push_back(ipc::value("Source not found."));
		return;
	}

	rval.push_back(ipc::value((uint64_t)ErrorCode::Ok));
	rval.push_back(ipc::value(args[0].value_union.ui64));
	rval.push_back(ipc::value((uint64_t)reinterpret_cast<intptr_t>(item)));
	return;
}

void osn::Scene::MoveItem(void* data, const int64_t id, const std::vector<ipc::value>& args, std::vector<ipc::value>& rval) {
	obs_source_t* source = osn::Source::GetInstance()->Get(args[0].value_union.ui64);
	if (!source) {
		rval.push_back(ipc::value((uint64_t)ErrorCode::InvalidReference));
		rval.push_back(ipc::value("Source reference is not valid."));
		return;
	}

	obs_scene_t* scene = obs_scene_from_source(source);
	if (!scene) {
		rval.push_back(ipc::value((uint64_t)ErrorCode::InvalidReference));
		rval.push_back(ipc::value("Source reference is not a scene."));
		return;
	}

	obs_sceneitem_t* item = reinterpret_cast<obs_sceneitem_t*>(args[1].value_union.ui64);
	obs_sceneitem_set_order_position(item, args[2].value_union.i32);

	rval.push_back(ipc::value((uint64_t)ErrorCode::Ok));
	return;
}

void osn::Scene::GetItem(void* data, const int64_t id, const std::vector<ipc::value>& args, std::vector<ipc::value>& rval) {
	obs_source_t* source = osn::Source::GetInstance()->Get(args[0].value_union.ui64);
	if (!source) {
		rval.push_back(ipc::value((uint64_t)ErrorCode::InvalidReference));
		rval.push_back(ipc::value("Source reference is not valid."));
		return;
	}

	obs_scene_t* scene = obs_scene_from_source(source);
	if (!scene) {
		rval.push_back(ipc::value((uint64_t)ErrorCode::InvalidReference));
		rval.push_back(ipc::value("Source reference is not a scene."));
		return;
	}

	struct EnumData {
		obs_sceneitem_t* item = nullptr;
		size_t findindex = 0;
		size_t index = 0;
	} ed;
	ed.findindex = args[1].value_union.ui64;

	auto cb = [](obs_scene_t* scene, obs_sceneitem_t* item, void* data) {
		EnumData* items = reinterpret_cast<EnumData*>(data);
		if (items->index == items->findindex) {
			items->item = item;
			return false;
		}
		items->index++;
		return true;
	};
	obs_scene_enum_items(scene, cb, &ed);

	if (!ed.item) {
		rval.push_back(ipc::value((uint64_t)ErrorCode::OutOfBounds));
		rval.push_back(ipc::value("Index not found in Scene."));
		return;
	}

	rval.push_back(ipc::value((uint64_t)ErrorCode::Ok));
	rval.push_back(ipc::value(args[0].value_union.ui64));
	rval.push_back(ipc::value((uint64_t)reinterpret_cast<intptr_t>(ed.item)));
}

void osn::Scene::GetItems(void* data, const int64_t id, const std::vector<ipc::value>& args, std::vector<ipc::value>& rval) {
	obs_source_t* source = osn::Source::GetInstance()->Get(args[0].value_union.ui64);
	if (!source) {
		rval.push_back(ipc::value((uint64_t)ErrorCode::InvalidReference));
		rval.push_back(ipc::value("Source reference is not valid."));
		return;
	}

	obs_scene_t* scene = obs_scene_from_source(source);
	if (!scene) {
		rval.push_back(ipc::value((uint64_t)ErrorCode::InvalidReference));
		rval.push_back(ipc::value("Source reference is not a scene."));
		return;
	}

	std::list<obs_sceneitem_t*> items;
	auto cb = [](obs_scene_t* scene, obs_sceneitem_t* item, void* data) {
		std::list<obs_sceneitem_t*>* items = reinterpret_cast<std::list<obs_sceneitem_t*>*>(data);
		items->push_back(item);
		return true;
	};
	obs_scene_enum_items(scene, cb, &items);
	
	rval.push_back(ipc::value((uint64_t)ErrorCode::Ok));
	for (obs_sceneitem_t* item : items) {
		rval.push_back(ipc::value((uint64_t)reinterpret_cast<intptr_t>(item)));
	}
}

void osn::Scene::GetItemsInRange(void* data, const int64_t id, const std::vector<ipc::value>& args, std::vector<ipc::value>& rval) {
	obs_source_t* source = osn::Source::GetInstance()->Get(args[0].value_union.ui64);
	if (!source) {
		rval.push_back(ipc::value((uint64_t)ErrorCode::InvalidReference));
		rval.push_back(ipc::value("Source reference is not valid."));
		return;
	}

	obs_scene_t* scene = obs_scene_from_source(source);
	if (!scene) {
		rval.push_back(ipc::value((uint64_t)ErrorCode::InvalidReference));
		rval.push_back(ipc::value("Source reference is not a scene."));
		return;
	}

	struct EnumData {
		std::list<obs_sceneitem_t*> items;
		size_t index_from = 0, index_to = 0;
		size_t index = 0;
	} ed;
	ed.index_from = args[1].value_union.ui64;
	ed.index_to = args[1].value_union.ui64;

	auto cb = [](obs_scene_t* scene, obs_sceneitem_t* item, void* data) {
		EnumData* ed = reinterpret_cast<EnumData*>(data);
		if ((ed->index >= ed->index_from) && (ed->index <= ed->index_to)) {
			ed->items.push_back(item);
		} else if (ed->index > ed->index_to) {
			return false;
		}
		ed->index++;
		return true;
	};
	obs_scene_enum_items(scene, cb, &ed);

	rval.push_back(ipc::value((uint64_t)ErrorCode::Ok));
	for (obs_sceneitem_t* item : ed.items) {
		rval.push_back(ipc::value((uint64_t)reinterpret_cast<intptr_t>(item)));
	}
}

void osn::Scene::Connect(void* data, const int64_t id, const std::vector<ipc::value>& args, std::vector<ipc::value>& rval) {
	// !FIXME! Signals
}

void osn::Scene::Disconnect(void* data, const int64_t id, const std::vector<ipc::value>& args, std::vector<ipc::value>& rval) {
	// !FIXME! Signals
}
