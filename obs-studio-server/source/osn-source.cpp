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

#include "osn-source.hpp"
#include "osn-common.hpp"
#include "error.hpp"
#include <ipc-server.hpp>
#include <ipc-class.hpp>
#include <ipc-function.hpp>
#include <ipc-value.hpp>
#include <map>
#include <memory>
#include <obs.h>
#include "obs-property.hpp"

osn::Source::SingletonObjectManager::SingletonObjectManager() {

}

osn::Source::SingletonObjectManager::~SingletonObjectManager() {

}

static osn::Source::SingletonObjectManager *somInstance;

bool osn::Source::Initialize() {
	if (somInstance)
		return false;
	somInstance = new SingletonObjectManager();
	return true;
}

bool osn::Source::Finalize() {
	if (!somInstance)
		return false;
	delete somInstance;
	somInstance = nullptr;
	return true;

}

osn::Source::SingletonObjectManager* osn::Source::GetInstance() {
	return somInstance;
}

void osn::Source::Register(ipc::server& srv) {
	std::shared_ptr<ipc::collection> cls = std::make_shared<ipc::collection>("Source");
	cls->register_function(std::make_shared<ipc::function>("GetDefaults", std::vector<ipc::type>{ipc::type::String}, GetSettings));
	cls->register_function(std::make_shared<ipc::function>("GetProperties", std::vector<ipc::type>{ipc::type::String}, GetTypeProperties));
	cls->register_function(std::make_shared<ipc::function>("GetOutputFlags", std::vector<ipc::type>{ipc::type::String}, GetTypeOutputFlags));

	cls->register_function(std::make_shared<ipc::function>("Remove", std::vector<ipc::type>{ipc::type::UInt64}, Remove));
	cls->register_function(std::make_shared<ipc::function>("Release", std::vector<ipc::type>{ipc::type::UInt64}, Release));
	cls->register_function(std::make_shared<ipc::function>("IsConfigurable", std::vector<ipc::type>{ipc::type::UInt64}, IsConfigurable));
	cls->register_function(std::make_shared<ipc::function>("GetProperties", std::vector<ipc::type>{ipc::type::UInt64}, GetProperties));
	cls->register_function(std::make_shared<ipc::function>("GetSettings", std::vector<ipc::type>{ipc::type::UInt64}, GetSettings));
	cls->register_function(std::make_shared<ipc::function>("Load", std::vector<ipc::type>{ipc::type::UInt64}, Load));
	cls->register_function(std::make_shared<ipc::function>("Save", std::vector<ipc::type>{ipc::type::UInt64}, Save));
	cls->register_function(std::make_shared<ipc::function>("Update", std::vector<ipc::type>{ipc::type::UInt64}, Update));
	cls->register_function(std::make_shared<ipc::function>("GetType", std::vector<ipc::type>{ipc::type::UInt64}, GetType));
	cls->register_function(std::make_shared<ipc::function>("GetName", std::vector<ipc::type>{ipc::type::UInt64}, GetName));
	cls->register_function(std::make_shared<ipc::function>("SetName", std::vector<ipc::type>{ipc::type::UInt64}, SetName));
	cls->register_function(std::make_shared<ipc::function>("GetOutputFlags", std::vector<ipc::type>{ipc::type::UInt64}, GetOutputFlags));
	cls->register_function(std::make_shared<ipc::function>("GetFlags", std::vector<ipc::type>{ipc::type::UInt64}, GetFlags));
	cls->register_function(std::make_shared<ipc::function>("SetFlags", std::vector<ipc::type>{ipc::type::UInt64}, SetFlags));
	cls->register_function(std::make_shared<ipc::function>("GetStatus", std::vector<ipc::type>{ipc::type::UInt64}, GetStatus));
	cls->register_function(std::make_shared<ipc::function>("GetId", std::vector<ipc::type>{ipc::type::UInt64}, GetId));
	cls->register_function(std::make_shared<ipc::function>("GetMuted", std::vector<ipc::type>{ipc::type::UInt64}, GetMuted));
	cls->register_function(std::make_shared<ipc::function>("SetMuted", std::vector<ipc::type>{ipc::type::UInt64, ipc::type::Int32}, SetMuted));
	cls->register_function(std::make_shared<ipc::function>("GetEnabled", std::vector<ipc::type>{ipc::type::UInt64}, GetEnabled));
	cls->register_function(std::make_shared<ipc::function>("SetEnabled", std::vector<ipc::type>{ipc::type::UInt64, ipc::type::Int32}, SetEnabled));
	srv.register_collection(cls);
}

void osn::Source::GetTypeProperties(void* data, const int64_t id, const std::vector<ipc::value>& args, std::vector<ipc::value>& rval) {
	// Per Type Properties (doesn't have an object).
	//obs_get_source_properties();
}

void osn::Source::GetTypeDefaults(void* data, const int64_t id, const std::vector<ipc::value>& args, std::vector<ipc::value>& rval) {
	// Per Type Defaults (doesn't have an object)
	//obs_get_source_defaults();
}

void osn::Source::GetTypeOutputFlags(void* data, const int64_t id, const std::vector<ipc::value>& args, std::vector<ipc::value>& rval) {
	// Per Type Defaults (doesn't have an object)
	//obs_get_source_output_flags();
}

void osn::Source::Remove(void* data, const int64_t id, const std::vector<ipc::value>& args, std::vector<ipc::value>& rval) {
	// Attempt to find the source asked to load.
	obs_source_t* src = osn::Source::GetInstance()->Get(args[0].value_union.ui64);
	if (src == nullptr) {
		rval.push_back(ipc::value((uint64_t)ErrorCode::InvalidReference));
		rval.push_back(ipc::value("Source reference is not valid."));
		return;
	}

	obs_source_remove(src);
	osn::Source::GetInstance()->Free(args[0].value_union.ui64);

	rval.push_back(ipc::value((uint64_t)ErrorCode::Ok));
	return;
}

void osn::Source::Release(void* data, const int64_t id, const std::vector<ipc::value>& args, std::vector<ipc::value>& rval) {
	// Attempt to find the source asked to load.
	obs_source_t* src = osn::Source::GetInstance()->Get(args[0].value_union.ui64);
	if (src == nullptr) {
		rval.push_back(ipc::value((uint64_t)ErrorCode::InvalidReference));
		rval.push_back(ipc::value("Source reference is not valid."));
		return;
	}

	obs_source_release(src);
	if (obs_source_removed(src)) {
		osn::Source::GetInstance()->Free(args[0].value_union.ui64);
	}

	rval.push_back(ipc::value((uint64_t)ErrorCode::Ok));
	return;
}

void osn::Source::IsConfigurable(void* data, const int64_t id, const std::vector<ipc::value>& args, std::vector<ipc::value>& rval) {
	// Attempt to find the source asked to load.
	obs_source_t* src = osn::Source::GetInstance()->Get(args[0].value_union.ui64);
	if (src == nullptr) {
		rval.push_back(ipc::value((uint64_t)ErrorCode::InvalidReference));
		rval.push_back(ipc::value("Source reference is not valid."));
		return;
	}

	rval.push_back(ipc::value((uint64_t)ErrorCode::Ok));
	rval.push_back(ipc::value(obs_source_configurable(src)));
	return;
}

void osn::Source::GetProperties(void* data, const int64_t id, const std::vector<ipc::value>& args, std::vector<ipc::value>& rval) {
	// Attempt to find the source asked to load.
	obs_source_t* src = osn::Source::GetInstance()->Get(args[0].value_union.ui64);
	if (src == nullptr) {
		rval.push_back(ipc::value((uint64_t)ErrorCode::InvalidReference));
		rval.push_back(ipc::value("Source reference is not valid."));
		return;
	}

	obs_properties_t* prp = obs_source_properties(src);
	const char* buf;

	rval.push_back(ipc::value((uint64_t)ErrorCode::Ok));
	for (obs_property_t* p = obs_properties_first(prp); (p != nullptr) && obs_property_next(&p);) {
		std::shared_ptr<obs::Property> prop;

		switch (obs_property_get_type(p)) {
			case OBS_PROPERTY_BOOL:
				prop = std::make_shared<obs::BooleanProperty>();
				break;
			case OBS_PROPERTY_INT: {
				auto prop2 = std::make_shared<obs::IntegerProperty>();
				prop2->field_type = obs::NumberProperty::NumberType(obs_property_int_type(p));
				prop2->minimum = obs_property_int_min(p);
				prop2->maximum = obs_property_int_max(p);
				prop2->step = obs_property_int_step(p);
				prop = prop2;
				break;
			}
			case OBS_PROPERTY_FLOAT: {
				auto prop2 = std::make_shared<obs::FloatProperty>();
				prop2->field_type = obs::NumberProperty::NumberType(obs_property_int_type(p));
				prop2->minimum = obs_property_float_min(p);
				prop2->maximum = obs_property_float_max(p);
				prop2->step = obs_property_float_step(p);
				prop = prop2;
				break;
			}
			case OBS_PROPERTY_TEXT: {
				auto prop2 = std::make_shared<obs::TextProperty>();
				prop2->field_type = obs::TextProperty::TextType(obs_proprety_text_type(p));
				prop = prop2;
				break;
			}
			case OBS_PROPERTY_PATH: {
				auto prop2 = std::make_shared<obs::PathProperty>();
				prop2->field_type = obs::PathProperty::PathType(obs_property_path_type(p));
				prop2->filter = (buf = obs_property_path_filter(p)) != nullptr ? buf : "";
				prop2->default_path = (buf = obs_property_path_default_path(p)) != nullptr ? buf : "";
				prop = prop2;
				break;
			}
			case OBS_PROPERTY_LIST: {
				auto prop2 = std::make_shared<obs::ListProperty>();
				prop2->field_type = obs::ListProperty::ListType(obs_property_list_type(p));
				prop2->format = obs::ListProperty::Format(obs_property_list_format(p));
				size_t items = obs_property_list_item_count(p);
				for (size_t idx = 0; idx < items; ++idx) {
					obs::ListProperty::Item entry;
					entry.name = (buf = obs_property_list_item_name(p, idx)) != nullptr ? buf : "";
					entry.enabled = !obs_property_list_item_disabled(p, idx);
					switch (prop2->format) {
						case obs::ListProperty::Format::Integer:
							entry.value_int = obs_property_list_item_int(p, idx);
							break;
						case obs::ListProperty::Format::Float:
							entry.value_float = obs_property_list_item_float(p, idx);
							break;
						case obs::ListProperty::Format::String:
							entry.value_string = (buf = obs_property_list_item_string(p, idx)) != nullptr ? buf : "";
							break;
					}
					prop2->items.push_back(std::move(entry));
				}
				prop = prop2;
				break;
			}
			case OBS_PROPERTY_COLOR:
				prop = std::make_shared<obs::ColorProperty>();
				break;
			case OBS_PROPERTY_BUTTON:
				prop = std::make_shared<obs::ButtonProperty>();
				break;
			case OBS_PROPERTY_FONT:
				prop = std::make_shared<obs::FontProperty>();
				break;
			case OBS_PROPERTY_EDITABLE_LIST: {
				auto prop2 = std::make_shared<obs::EditableListProperty>();
				prop2->field_type = obs::EditableListProperty::ListType(obs_property_editable_list_type(p));
				prop2->filter = (buf = obs_property_editable_list_filter(p)) != nullptr ? buf : "";
				prop2->default_path = (buf = obs_property_editable_list_default_path(p)) != nullptr ? buf : "";
				prop = prop2;
				break;
			}
			case OBS_PROPERTY_FRAME_RATE: {
				auto prop2 = std::make_shared<obs::FrameRateProperty>();
				size_t num_ranges = obs_property_frame_rate_fps_ranges_count(p);
				for (size_t idx = 0; idx < num_ranges; idx++) {
					auto min = obs_property_frame_rate_fps_range_min(p, idx),
						max = obs_property_frame_rate_fps_range_max(p, idx);

					obs::FrameRateProperty::Range range;
					range.minimum.first = min.numerator;
					range.minimum.second = min.denominator;
					range.maximum.first = max.numerator;
					range.maximum.second = max.denominator;

					prop2->ranges.push_back(std::move(range));
				}

				size_t num_options = obs_property_frame_rate_options_count(p);
				for (size_t idx = 0; idx < num_options; idx++) {
					auto min = obs_property_frame_rate_fps_range_min(p, idx),
						max = obs_property_frame_rate_fps_range_max(p, idx);

					obs::FrameRateProperty::Option option;
					option.name = (buf = obs_property_frame_rate_option_name(p, idx)) != nullptr ? buf : "";
					option.description = (buf = obs_property_frame_rate_option_description(p, idx)) != nullptr ? buf : "";

					prop2->options.push_back(std::move(option));
				}

				prop = prop2;
				break;
			}
		}

		if (!prop) {
			continue;
		}

		prop->name = obs_property_name(p);
		prop->description = obs_property_description(p) ? obs_property_description(p) : "";
		prop->long_description = obs_property_long_description(p) ? obs_property_long_description(p) : "";
		prop->enabled = obs_property_enabled(p);
		prop->visible = obs_property_visible(p);

		std::vector<char> buf(prop->size());
		if (prop->serialize(buf)) {
			rval.push_back(ipc::value(buf));
		}
	}
	return;
}

void osn::Source::GetSettings(void* data, const int64_t id, const std::vector<ipc::value>& args, std::vector<ipc::value>& rval) {
	// Attempt to find the source asked to load.
	obs_source_t* src = osn::Source::GetInstance()->Get(args[0].value_union.ui64);
	if (src == nullptr) {
		rval.push_back(ipc::value((uint64_t)ErrorCode::InvalidReference));
		rval.push_back(ipc::value("Source reference is not valid."));
		return;
	}

	obs_data_t* sets = obs_source_get_settings(src);
	rval.push_back(ipc::value((uint64_t)ErrorCode::Ok));
	rval.push_back(ipc::value(obs_data_get_json(sets)));
	obs_data_release(sets);
	return;
}

void osn::Source::Update(void* data, const int64_t id, const std::vector<ipc::value>& args, std::vector<ipc::value>& rval) {
	// Attempt to find the source asked to load.
	obs_source_t* src = osn::Source::GetInstance()->Get(args[0].value_union.ui64);
	if (src == nullptr) {
		rval.push_back(ipc::value((uint64_t)ErrorCode::InvalidReference));
		rval.push_back(ipc::value("Source reference is not valid."));
		return;
	}

	obs_data_t* sets = obs_data_create_from_json(args[1].value_str.c_str());
	obs_source_update(src, sets);
	obs_data_release(sets);

	rval.push_back(ipc::value((uint64_t)ErrorCode::Ok));
	return;
}

void osn::Source::Load(void* data, const int64_t id, const std::vector<ipc::value>& args, std::vector<ipc::value>& rval) {
	// Attempt to find the source asked to load.
	obs_source_t* src = osn::Source::GetInstance()->Get(args[0].value_union.ui64);
	if (src == nullptr) {
		rval.push_back(ipc::value((uint64_t)ErrorCode::InvalidReference));
		rval.push_back(ipc::value("Source reference is not valid."));
		return;
	}

	obs_source_load(src);

	rval.push_back(ipc::value((uint64_t)ErrorCode::Ok));
	return;
}

void osn::Source::Save(void* data, const int64_t id, const std::vector<ipc::value>& args, std::vector<ipc::value>& rval) {
	// Attempt to find the source asked to load.
	obs_source_t* src = osn::Source::GetInstance()->Get(args[0].value_union.ui64);
	if (src == nullptr) {
		rval.push_back(ipc::value((uint64_t)ErrorCode::InvalidReference));
		rval.push_back(ipc::value("Source reference is not valid."));
		return;
	}

	obs_source_save(src);

	rval.push_back(ipc::value((uint64_t)ErrorCode::Ok));
	return;
}

void osn::Source::GetType(void* data, const int64_t id, const std::vector<ipc::value>& args, std::vector<ipc::value>& rval) {
	// Attempt to find the source asked to load.
	obs_source_t* src = osn::Source::GetInstance()->Get(args[0].value_union.ui64);
	if (src == nullptr) {
		rval.push_back(ipc::value((uint64_t)ErrorCode::InvalidReference));
		rval.push_back(ipc::value("Source reference is not valid."));
		return;
	}

	rval.push_back(ipc::value((uint64_t)ErrorCode::Ok));
	rval.push_back(ipc::value(obs_source_get_type(src)));
	return;
}

void osn::Source::GetName(void* data, const int64_t id, const std::vector<ipc::value>& args, std::vector<ipc::value>& rval) {
	// Attempt to find the source asked to load.
	obs_source_t* src = osn::Source::GetInstance()->Get(args[0].value_union.ui64);
	if (src == nullptr) {
		rval.push_back(ipc::value((uint64_t)ErrorCode::InvalidReference));
		rval.push_back(ipc::value("Source reference is not valid."));
		return;
	}

	rval.push_back(ipc::value((uint64_t)ErrorCode::Ok));
	rval.push_back(ipc::value(obs_source_get_name(src)));
	return;
}

void osn::Source::SetName(void* data, const int64_t id, const std::vector<ipc::value>& args, std::vector<ipc::value>& rval) {
	// Attempt to find the source asked to load.
	obs_source_t* src = osn::Source::GetInstance()->Get(args[0].value_union.ui64);
	if (src == nullptr) {
		rval.push_back(ipc::value((uint64_t)ErrorCode::InvalidReference));
		rval.push_back(ipc::value("Source reference is not valid."));
		return;
	}

	obs_source_set_name(src, args[1].value_str.c_str());

	rval.push_back(ipc::value((uint64_t)ErrorCode::Ok));
	rval.push_back(ipc::value(obs_source_get_name(src)));
	return;
}

void osn::Source::GetOutputFlags(void* data, const int64_t id, const std::vector<ipc::value>& args, std::vector<ipc::value>& rval) {
	// Attempt to find the source asked to load.
	obs_source_t* src = osn::Source::GetInstance()->Get(args[0].value_union.ui64);
	if (src == nullptr) {
		rval.push_back(ipc::value((uint64_t)ErrorCode::InvalidReference));
		rval.push_back(ipc::value("Source reference is not valid."));
		return;
	}

	rval.push_back(ipc::value((uint64_t)ErrorCode::Ok));
	rval.push_back(ipc::value(obs_source_get_output_flags(src)));
	return;
}

void osn::Source::GetFlags(void* data, const int64_t id, const std::vector<ipc::value>& args, std::vector<ipc::value>& rval) {
	// Attempt to find the source asked to load.
	obs_source_t* src = osn::Source::GetInstance()->Get(args[0].value_union.ui64);
	if (src == nullptr) {
		rval.push_back(ipc::value((uint64_t)ErrorCode::InvalidReference));
		rval.push_back(ipc::value("Source reference is not valid."));
		return;
	}

	rval.push_back(ipc::value((uint64_t)ErrorCode::Ok));
	rval.push_back(ipc::value(obs_source_get_flags(src)));
	return;
}

void osn::Source::SetFlags(void* data, const int64_t id, const std::vector<ipc::value>& args, std::vector<ipc::value>& rval) {
	// Attempt to find the source asked to load.
	obs_source_t* src = osn::Source::GetInstance()->Get(args[0].value_union.ui64);
	if (src == nullptr) {
		rval.push_back(ipc::value((uint64_t)ErrorCode::InvalidReference));
		rval.push_back(ipc::value("Source reference is not valid."));
		return;
	}

	obs_source_set_flags(src, args[1].value_union.ui32);

	rval.push_back(ipc::value((uint64_t)ErrorCode::Ok));
	rval.push_back(ipc::value(obs_source_get_flags(src)));
	return;
}

void osn::Source::GetStatus(void* data, const int64_t id, const std::vector<ipc::value>& args, std::vector<ipc::value>& rval) {
	// Attempt to find the source asked to load.
	obs_source_t* src = osn::Source::GetInstance()->Get(args[0].value_union.ui64);
	if (src == nullptr) {
		rval.push_back(ipc::value((uint64_t)ErrorCode::InvalidReference));
		rval.push_back(ipc::value("Source reference is not valid."));
		return;
	}

	rval.push_back(ipc::value((uint64_t)ErrorCode::Ok));
	rval.push_back(ipc::value((uint64_t)true));
	return;
}

void osn::Source::GetId(void* data, const int64_t id, const std::vector<ipc::value>& args, std::vector<ipc::value>& rval) {
	// Attempt to find the source asked to load.
	obs_source_t* src = osn::Source::GetInstance()->Get(args[0].value_union.ui64);
	if (src == nullptr) {
		rval.push_back(ipc::value((uint64_t)ErrorCode::InvalidReference));
		rval.push_back(ipc::value("Source reference is not valid."));
		return;
	}

	rval.push_back(ipc::value((uint64_t)ErrorCode::Ok));
	const char* sid = obs_source_get_id(src);
	rval.push_back(ipc::value(sid ? sid : ""));
	return;
}

void osn::Source::GetMuted(void* data, const int64_t id, const std::vector<ipc::value>& args, std::vector<ipc::value>& rval) {
	// Attempt to find the source asked to load.
	obs_source_t* src = osn::Source::GetInstance()->Get(args[0].value_union.ui64);
	if (src == nullptr) {
		rval.push_back(ipc::value((uint64_t)ErrorCode::InvalidReference));
		rval.push_back(ipc::value("Source reference is not valid."));
		return;
	}

	rval.push_back(ipc::value((uint64_t)ErrorCode::Ok));
	rval.push_back(ipc::value(obs_source_muted(src)));
	return;
}

void osn::Source::SetMuted(void* data, const int64_t id, const std::vector<ipc::value>& args, std::vector<ipc::value>& rval) {
	// Attempt to find the source asked to load.
	obs_source_t* src = osn::Source::GetInstance()->Get(args[0].value_union.ui64);
	if (src == nullptr) {
		rval.push_back(ipc::value((uint64_t)ErrorCode::InvalidReference));
		rval.push_back(ipc::value("Source reference is not valid."));
		return;
	}

	obs_source_set_muted(src, !!args[1].value_union.i32);

	rval.push_back(ipc::value((uint64_t)ErrorCode::Ok));
	rval.push_back(ipc::value(obs_source_muted(src)));
	return;
}

void osn::Source::GetEnabled(void* data, const int64_t id, const std::vector<ipc::value>& args, std::vector<ipc::value>& rval) {
	// Attempt to find the source asked to load.
	obs_source_t* src = osn::Source::GetInstance()->Get(args[0].value_union.ui64);
	if (src == nullptr) {
		rval.push_back(ipc::value((uint64_t)ErrorCode::InvalidReference));
		rval.push_back(ipc::value("Source reference is not valid."));
		return;
	}

	rval.push_back(ipc::value((uint64_t)ErrorCode::Ok));
	rval.push_back(ipc::value(obs_source_enabled(src)));
	return;
}

void osn::Source::SetEnabled(void* data, const int64_t id, const std::vector<ipc::value>& args, std::vector<ipc::value>& rval) {
	// Attempt to find the source asked to load.
	obs_source_t* src = osn::Source::GetInstance()->Get(args[0].value_union.ui64);
	if (src == nullptr) {
		rval.push_back(ipc::value((uint64_t)ErrorCode::InvalidReference));
		rval.push_back(ipc::value("Source reference is not valid."));
		return;
	}

	obs_source_set_enabled(src, !!args[1].value_union.i32);

	rval.push_back(ipc::value((uint64_t)ErrorCode::Ok));
	rval.push_back(ipc::value(obs_source_enabled(src)));
	return;
}
