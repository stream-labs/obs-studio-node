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

#include "osn-IEncoder.hpp"
#include <obs.h>

void osn::IEncoder::Register(IPC::Server& srv) {
	auto cls = std::make_shared<IPC::Class>("IEncoder");
	cls->RegisterFunction(std::make_shared<IPC::Function>("GetId", std::vector<IPC::Type>{IPC::Type::String}, &GetId));
	cls->RegisterFunction(std::make_shared<IPC::Function>("GetName", std::vector<IPC::Type>{IPC::Type::String}, &GetName));
	cls->RegisterFunction(std::make_shared<IPC::Function>("SetName", std::vector<IPC::Type>{IPC::Type::String, IPC::Type::String}, &SetName));
	cls->RegisterFunction(std::make_shared<IPC::Function>("GetCaps", std::vector<IPC::Type>{IPC::Type::String}, &GetCaps));
	cls->RegisterFunction(std::make_shared<IPC::Function>("GetType", std::vector<IPC::Type>{IPC::Type::String}, &GetType));
	cls->RegisterFunction(std::make_shared<IPC::Function>("GetCodec", std::vector<IPC::Type>{IPC::Type::String}, &GetCodec));
	cls->RegisterFunction(std::make_shared<IPC::Function>("Update", std::vector<IPC::Type>{IPC::Type::String, IPC::Type::String}, &Update));
	cls->RegisterFunction(std::make_shared<IPC::Function>("GetProperties", std::vector<IPC::Type>{IPC::Type::String}, &GetProperties));
	cls->RegisterFunction(std::make_shared<IPC::Function>("GetSettings", std::vector<IPC::Type>{IPC::Type::String}, &GetSettings));
	cls->RegisterFunction(std::make_shared<IPC::Function>("Release", std::vector<IPC::Type>{IPC::Type::String}, &Release));
	srv.RegisterClass(cls);
}

void osn::IEncoder::GetId(void* data, const int64_t id, const std::vector<IPC::Value>& args, std::vector<IPC::Value>& rval) {
	auto p = obs_get_encoder_by_name(args[0].value_str.c_str());
	if (p == nullptr) {
		rval[0].type = IPC::Type::Null;
		rval[0].value_str = "Unable to find encoder.";
		return;
	}

	rval.resize(1);
	rval[0].type = IPC::Type::String;
	rval[0].value_str = obs_encoder_get_id(p);
	obs_encoder_release(p);
}

void osn::IEncoder::GetName(void* data, const int64_t id, const std::vector<IPC::Value>& args, std::vector<IPC::Value>& rval) {
	auto p = obs_get_encoder_by_name(args[0].value_str.c_str());
	if (p == nullptr) {
		rval[0].type = IPC::Type::Null;
		rval[0].value_str = "Unable to find encoder.";
		return;
	}

	rval[0].type = IPC::Type::String;
	rval[0].value_str = obs_encoder_get_name(p);
	obs_encoder_release(p);
}

void osn::IEncoder::SetName(void* data, const int64_t id, const std::vector<IPC::Value>& args, std::vector<IPC::Value>& rval) {
	auto p = obs_get_encoder_by_name(args[0].value_str.c_str());
	if (p == nullptr) {
		rval[0].type = IPC::Type::Null;
		rval[0].value_str = "Unable to find encoder.";
		return;
	}

	obs_encoder_set_name(p, args[0].value_str.c_str());
	obs_encoder_release(p);
}

void osn::IEncoder::GetCaps(void* data, const int64_t id, const std::vector<IPC::Value>& args, std::vector<IPC::Value>& rval) {
	rval[0].type = IPC::Type::UInt32;
	rval[0].value.ui32 = obs_get_encoder_caps(args[0].value_str.c_str());
}

void osn::IEncoder::GetType(void* data, const int64_t id, const std::vector<IPC::Value>& args, std::vector<IPC::Value>& rval) {
	auto p = obs_get_encoder_by_name(args[0].value_str.c_str());
	if (p == nullptr) {
		rval[0].type = IPC::Type::Null;
		rval[0].value_str = "Unable to find encoder.";
		return;
	}

	rval[0].type = IPC::Type::Int32;
	rval[0].value.i32 = obs_encoder_get_type(p);
	obs_encoder_release(p);
}

void osn::IEncoder::GetCodec(void* data, const int64_t id, const std::vector<IPC::Value>& args, std::vector<IPC::Value>& rval) {
	auto p = obs_get_encoder_by_name(args[0].value_str.c_str());
	if (p == nullptr) {
		rval[0].type = IPC::Type::Null;
		rval[0].value_str = "Unable to find encoder.";
		return;
	}

	rval[0].type = IPC::Type::String;
	rval[0].value_str = obs_encoder_get_codec(p);
	obs_encoder_release(p);
}

void osn::IEncoder::Update(void* data, const int64_t id, const std::vector<IPC::Value>& args, std::vector<IPC::Value>& rval) {

}

void osn::IEncoder::GetProperties(void* data, const int64_t id, const std::vector<IPC::Value>& args, std::vector<IPC::Value>& rval) {

}

void osn::IEncoder::GetSettings(void* data, const int64_t id, const std::vector<IPC::Value>& args, std::vector<IPC::Value>& rval) {

}

void osn::IEncoder::Release(void* data, const int64_t id, const std::vector<IPC::Value>& args, std::vector<IPC::Value>& rval) {
	auto p = obs_get_encoder_by_name(args[0].value_str.c_str());
	if (p == nullptr) {
		rval[0].type = IPC::Type::Null;
		rval[0].value_str = "Unable to find encoder.";
		return;
	}

	obs_encoder_release(p);
	obs_encoder_release(p);
}
