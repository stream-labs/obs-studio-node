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

#include "osn-common.hpp"

void osn::common::OBSDataItemToIPCValue(obs_data_item_t* itm, std::vector<IPC::Value>& arr) {
	obs_data_type dt = obs_data_item_gettype(itm);
	bool hasUserValue = obs_data_item_has_user_value(itm),
		hasDefaultValue = obs_data_item_has_default_value(itm),
		hasAutoSelectValue = obs_data_item_has_autoselect_value(itm);

	arr.push_back(IPC::Value(obs_data_item_get_name(itm)));
	arr.push_back(IPC::Value(dt));
	arr.push_back(IPC::Value(hasUserValue));
	arr.push_back(IPC::Value(hasDefaultValue));
	arr.push_back(IPC::Value(hasAutoSelectValue));
	switch (dt) {
		case OBS_DATA_BOOLEAN:
			if (hasUserValue) {
				arr.push_back(IPC::Value(obs_data_item_get_bool(itm)));
			}
			if (hasDefaultValue) {
				arr.push_back(IPC::Value(obs_data_item_get_default_bool(itm)));
			}
			if (hasAutoSelectValue) {
				arr.push_back(IPC::Value(obs_data_item_get_autoselect_bool(itm)));
			}
			break;
		case OBS_DATA_NUMBER:
			obs_data_number_type nt = obs_data_item_numtype(itm);
			arr.push_back(IPC::Value(nt));
			switch (nt) {
				case OBS_DATA_NUM_INT:
					if (hasUserValue) {
						arr.push_back(IPC::Value(obs_data_item_get_int(itm)));
					}
					if (hasDefaultValue) {
						arr.push_back(IPC::Value(obs_data_item_get_default_int(itm)));
					}
					if (hasAutoSelectValue) {
						arr.push_back(IPC::Value(obs_data_item_get_autoselect_int(itm)));
					}
					break;
				case OBS_DATA_NUM_DOUBLE:
					if (hasUserValue) {
						arr.push_back(IPC::Value(obs_data_item_get_double(itm)));
					}
					if (hasDefaultValue) {
						arr.push_back(IPC::Value(obs_data_item_get_default_double(itm)));
					}
					if (hasAutoSelectValue) {
						arr.push_back(IPC::Value(obs_data_item_get_autoselect_double(itm)));
					}
					break;
			}
			break;
		case OBS_DATA_STRING:
			if (hasUserValue) {
				arr.push_back(IPC::Value(obs_data_item_get_string(itm)));
			}
			if (hasDefaultValue) {
				arr.push_back(IPC::Value(obs_data_item_get_default_string(itm)));
			}
			if (hasAutoSelectValue) {
				arr.push_back(IPC::Value(obs_data_item_get_autoselect_string(itm)));
			}
			break;
		case OBS_DATA_OBJECT:
			if (hasUserValue) {
				OBSDataToIPCValue(obs_data_item_get_obj(itm), arr);
			}
			if (hasDefaultValue) {
				OBSDataToIPCValue(obs_data_item_get_default_obj(itm), arr);
			}
			if (hasAutoSelectValue) {
				OBSDataToIPCValue(obs_data_item_get_autoselect_obj(itm), arr);
			}
			break;
		case OBS_DATA_ARRAY:
			if (hasUserValue) {
				OBSDataArrayToIPCValue(obs_data_item_get_array(itm), arr);
			}
			if (hasDefaultValue) {
				OBSDataArrayToIPCValue(obs_data_item_get_default_array(itm), arr);
			}
			if (hasAutoSelectValue) {
				OBSDataArrayToIPCValue(obs_data_item_get_autoselect_array(itm), arr);
			}
			break;
	}
}

void osn::common::OBSDataArrayToIPCValue(obs_data_array_t* val, std::vector<IPC::Value>& arr) {
	size_t n = obs_data_array_count(val);
	arr.push_back(n);
	for (size_t idx = 0; idx < n; idx++) {
		OBSDataToIPCValue(obs_data_array_item(val, idx));
	}
}

void osn::common::OBSDataToIPCValue(obs_data_t* val, std::vector<IPC::Value>& arr) {
	for (obs_data_item_t* itm = obs_data_first(val); itm != nullptr; obs_data_item_next(&itm)) {
		OBSDataItemToIPCValue(itm, arr);
	}
	arr.push_back(IPC::Value());
}

void osn::common::OBSDataItemFromIPCValue(obs_data_t* val, std::vector<IPC::Value>::const_iterator& it, std::vector<IPC::Value>::const_iterator& end) {
	if (it->type != IPC::Type::String) {
		throw std::runtime_error("Malformed IPC Data");
	}
	std::string name = it->value_str; it++;
	if (it->type != IPC::Type::Int32) {
		throw std::runtime_error("Malformed IPC Data");
	}
	obs_data_type dt = (obs_data_type)it->value.i32; it++;
	if (it->type != IPC::Type::Int32) {
		throw std::runtime_error("Malformed IPC Data");
	}
	bool hasUserValue = (bool)!!it->value.i32; it++;
	if (it->type != IPC::Type::Int32) {
		throw std::runtime_error("Malformed IPC Data");
	}
	bool hasDefaultValue = (bool)!!it->value.i32; it++;
	if (it->type != IPC::Type::Int32) {
		throw std::runtime_error("Malformed IPC Data");
	}
	bool hasAutoSelectValue = (bool)!!it->value.i32; it++;

	switch (dt) {
		case OBS_DATA_BOOLEAN:
			if (hasUserValue) {
				if (it->type != IPC::Type::Int32) {
					throw std::runtime_error("Malformed IPC Data");
				}
				obs_data_set_bool(val, name.c_str(), !!it->value.i32); it++;
			}
			if (hasDefaultValue) {
				if (it->type != IPC::Type::Int32) {
					throw std::runtime_error("Malformed IPC Data");
				}
				obs_data_set_default_bool(val, name.c_str(), !!it->value.i32); it++;
			}
			if (hasAutoSelectValue) {
				if (it->type != IPC::Type::Int32) {
					throw std::runtime_error("Malformed IPC Data");
				}
				obs_data_set_autoselect_bool(val, name.c_str(), !!it->value.i32); it++;
			}
			break;
		case OBS_DATA_NUMBER:
			if (it->type != IPC::Type::Int32) {
				throw std::runtime_error("Malformed IPC Data");
			}
			obs_data_number_type nt = (obs_data_number_type)it->value.i32; it++;
			switch (nt) {
				case OBS_DATA_NUM_INT:
					if (hasUserValue) {
						if (it->type != IPC::Type::Int64) {
							throw std::runtime_error("Malformed IPC Data");
						}
						obs_data_set_int(val, name.c_str(), it->value.i64); it++;
					}
					if (hasDefaultValue) {
						if (it->type != IPC::Type::Int64) {
							throw std::runtime_error("Malformed IPC Data");
						}
						obs_data_set_int(val, name.c_str(), it->value.i64); it++;
					}
					if (hasAutoSelectValue) {
						if (it->type != IPC::Type::Int64) {
							throw std::runtime_error("Malformed IPC Data");
						}
						obs_data_set_autoselect_int(val, name.c_str(), it->value.i64); it++;
					}
					break;
				case OBS_DATA_NUM_DOUBLE:
					if (hasUserValue) {
						if (it->type != IPC::Type::Double) {
							throw std::runtime_error("Malformed IPC Data");
						}
						obs_data_set_double(val, name.c_str(), it->value.fp64); it++;
					}
					if (hasDefaultValue) {
						if (it->type != IPC::Type::Double) {
							throw std::runtime_error("Malformed IPC Data");
						}
						obs_data_set_default_double(val, name.c_str(), it->value.fp64); it++;
					}
					if (hasAutoSelectValue) {
						if (it->type != IPC::Type::Double) {
							throw std::runtime_error("Malformed IPC Data");
						}
						obs_data_set_autoselect_double(val, name.c_str(), it->value.fp64); it++;
					}
					break;
			}
			break;
		case OBS_DATA_STRING:
			if (hasUserValue) {
				if (it->type != IPC::Type::String) {
					throw std::runtime_error("Malformed IPC Data");
				}
				obs_data_set_string(val, name.c_str(), it->value_str.c_str()); it++;
			}
			if (hasDefaultValue) {
				if (it->type != IPC::Type::String) {
					throw std::runtime_error("Malformed IPC Data");
				}
				obs_data_set_default_string(val, name.c_str(), it->value_str.c_str()); it++;
			}
			if (hasAutoSelectValue) {
				if (it->type != IPC::Type::String) {
					throw std::runtime_error("Malformed IPC Data");
				}
				obs_data_set_autoselect_string(val, name.c_str(), it->value_str.c_str()); it++;
			}
			break;
		case OBS_DATA_OBJECT:
			if (hasUserValue) {
				obs_data_t* nval = nullptr;
				OBSDataFromIPCValue(&nval, it, end);
				obs_data_set_obj(val, name.c_str(), nval);
			}
			if (hasDefaultValue) {
				obs_data_t* nval = nullptr;
				OBSDataFromIPCValue(&nval, it, end);
				obs_data_set_obj(val, name.c_str(), nval);
			}
			if (hasAutoSelectValue) {
				obs_data_t* nval = nullptr;
				OBSDataFromIPCValue(&nval, it, end);
				obs_data_set_obj(val, name.c_str(), nval);
			}
			break;
		case OBS_DATA_ARRAY:
			if (hasUserValue) {
				obs_data_array_t* nval = nullptr;
				OBSDataArrayFromIPCValue(nval, it, end);
				obs_data_set_array(val, name.c_str(), nval);
			}
			if (hasDefaultValue) {
				obs_data_array_t* nval = nullptr;
				OBSDataArrayFromIPCValue(nval, it, end);
				obs_data_set_array(val, name.c_str(), nval);
			}
			if (hasAutoSelectValue) {
				obs_data_array_t* nval = nullptr;
				OBSDataArrayFromIPCValue(nval, it, end);
				obs_data_set_array(val, name.c_str(), nval);
			}
			break;
	}
}

void osn::common::OBSDataFromIPCValue(obs_data_t** val, std::vector<IPC::Value>::const_iterator& begin, std::vector<IPC::Value>::const_iterator& end) {
	if (*val == nullptr) {
		*val = obs_data_create();
	}
	for (auto it = begin; it != end; it++) {
		if (it->type == IPC::Type::Null) {
			return;
		}
		OBSDataItemFromIPCValue(*val, it, end);
	}
}

void osn::common::OBSDataFromIPCValue(obs_data_t** val, std::vector<IPC::Value> const& arr) {
	OBSDataFromIPCValue(val, arr.begin(), arr.end());
}
