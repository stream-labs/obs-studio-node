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

#include "osn-IEncoder.hpp"
#include "error.hpp"
#include "utility.hpp"
#include <obs.h>

void osn::IEncoder::Register(ipc::server& srv)
{
	auto cls = std::make_shared<ipc::collection>("IEncoder");
	cls->register_function(std::make_shared<ipc::function>("GetId", std::vector<ipc::type>{ipc::type::String}, &GetId));
	cls->register_function(
	    std::make_shared<ipc::function>("GetName", std::vector<ipc::type>{ipc::type::String}, &GetName));
	cls->register_function(std::make_shared<ipc::function>(
	    "SetName", std::vector<ipc::type>{ipc::type::String, ipc::type::String}, &SetName));
	cls->register_function(
	    std::make_shared<ipc::function>("GetCaps", std::vector<ipc::type>{ipc::type::String}, &GetCaps));
	cls->register_function(
	    std::make_shared<ipc::function>("GetType", std::vector<ipc::type>{ipc::type::String}, &GetType));
	cls->register_function(
	    std::make_shared<ipc::function>("GetCodec", std::vector<ipc::type>{ipc::type::String}, &GetCodec));
	cls->register_function(std::make_shared<ipc::function>(
	    "Update", std::vector<ipc::type>{ipc::type::String, ipc::type::String}, &Update));
	cls->register_function(
	    std::make_shared<ipc::function>("GetProperties", std::vector<ipc::type>{ipc::type::String}, &GetProperties));
	cls->register_function(
	    std::make_shared<ipc::function>("GetSettings", std::vector<ipc::type>{ipc::type::String}, &GetSettings));
	cls->register_function(
	    std::make_shared<ipc::function>("Release", std::vector<ipc::type>{ipc::type::String}, &Release));
	srv.register_collection(cls);
}

void osn::IEncoder::GetId(
    void*                          data,
    const int64_t                  id,
    const std::vector<ipc::value>& args,
    std::vector<ipc::value>&       rval)
{
	auto p = obs_get_encoder_by_name(args[0].value_str.c_str());
	if (p == nullptr) {
		PRETTY_THROW("invalid reference");
	}

	rval.push_back(ipc::value((uint64_t)ErrorCode::Ok));
	rval.push_back(ipc::value(obs_encoder_get_id(p)));

	obs_encoder_release(p);
}

void osn::IEncoder::GetName(
    void*                          data,
    const int64_t                  id,
    const std::vector<ipc::value>& args,
    std::vector<ipc::value>&       rval)
{
	auto p = obs_get_encoder_by_name(args[0].value_str.c_str());
	if (p == nullptr) {
		PRETTY_THROW("invalid reference");
	}

	rval.push_back(ipc::value((uint64_t)ErrorCode::Ok));
	rval.push_back(ipc::value(obs_encoder_get_name(p)));

	obs_encoder_release(p);
}

void osn::IEncoder::SetName(
    void*                          data,
    const int64_t                  id,
    const std::vector<ipc::value>& args,
    std::vector<ipc::value>&       rval)
{
	auto p = obs_get_encoder_by_name(args[0].value_str.c_str());
	if (p == nullptr) {
		PRETTY_THROW("invalid reference");
	}

	rval.push_back(ipc::value((uint64_t)ErrorCode::Ok));
	obs_encoder_set_name(p, args[0].value_str.c_str());

	obs_encoder_release(p);
}

void osn::IEncoder::GetCaps(
    void*                          data,
    const int64_t                  id,
    const std::vector<ipc::value>& args,
    std::vector<ipc::value>&       rval)
{
	rval.push_back(ipc::value((uint64_t)ErrorCode::Ok));
	rval.push_back(ipc::value(uint32_t(obs_get_encoder_caps(args[0].value_str.c_str()))));
}

void osn::IEncoder::GetType(
    void*                          data,
    const int64_t                  id,
    const std::vector<ipc::value>& args,
    std::vector<ipc::value>&       rval)
{
	auto p = obs_get_encoder_by_name(args[0].value_str.c_str());
	if (p == nullptr) {
		PRETTY_THROW("invalid reference");
	}

	rval.push_back(ipc::value((uint64_t)ErrorCode::Ok));
	rval.push_back(ipc::value(int32_t(obs_encoder_get_type(p))));

	obs_encoder_release(p);
}

void osn::IEncoder::GetCodec(
    void*                          data,
    const int64_t                  id,
    const std::vector<ipc::value>& args,
    std::vector<ipc::value>&       rval)
{
	auto p = obs_get_encoder_by_name(args[0].value_str.c_str());
	if (p == nullptr) {
		PRETTY_THROW("invalid reference");
	}

	rval.push_back(ipc::value((uint64_t)ErrorCode::Ok));
	rval.push_back(ipc::value(int64_t(obs_encoder_get_codec(p))));

	obs_encoder_release(p);
}

void osn::IEncoder::Update(
    void*                          data,
    const int64_t                  id,
    const std::vector<ipc::value>& args,
    std::vector<ipc::value>&       rval)
{}

void osn::IEncoder::GetProperties(
    void*                          data,
    const int64_t                  id,
    const std::vector<ipc::value>& args,
    std::vector<ipc::value>&       rval)
{}

void osn::IEncoder::GetSettings(
    void*                          data,
    const int64_t                  id,
    const std::vector<ipc::value>& args,
    std::vector<ipc::value>&       rval)
{}

void osn::IEncoder::Release(
    void*                          data,
    const int64_t                  id,
    const std::vector<ipc::value>& args,
    std::vector<ipc::value>&       rval)
{
	auto p = obs_get_encoder_by_name(args[0].value_str.c_str());
	if (p == nullptr) {
		PRETTY_THROW("invalid reference");
	}

	rval.push_back(ipc::value((uint64_t)ErrorCode::Ok));

	obs_encoder_release(p);
}
