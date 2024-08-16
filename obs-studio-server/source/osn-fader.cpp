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

#include "osn-fader.hpp"
#include "osn-error.hpp"
#include "obs.h"
#include "osn-source.hpp"
#include "shared.hpp"
#include "utility.hpp"

osn::Fader::Manager &osn::Fader::Manager::GetInstance()
{
	static osn::Fader::Manager _inst;
	return _inst;
}

void osn::Fader::Register(ipc::server &srv)
{
	std::shared_ptr<ipc::collection> cls = std::make_shared<ipc::collection>("Fader");
	cls->register_function(std::make_shared<ipc::function>("Create", std::vector<ipc::type>{ipc::type::Int32}, Create));
	cls->register_function(std::make_shared<ipc::function>("Destroy", std::vector<ipc::type>{ipc::type::UInt64}, Destroy));
	cls->register_function(std::make_shared<ipc::function>("GetDeziBel", std::vector<ipc::type>{ipc::type::UInt64}, GetDeziBel));
	cls->register_function(std::make_shared<ipc::function>("SetDeziBel", std::vector<ipc::type>{ipc::type::UInt64, ipc::type::Float}, SetDeziBel));
	cls->register_function(std::make_shared<ipc::function>("GetDeflection", std::vector<ipc::type>{ipc::type::UInt64}, GetDeflection));
	cls->register_function(std::make_shared<ipc::function>("SetDeflection", std::vector<ipc::type>{ipc::type::UInt64, ipc::type::Float}, SetDeflection));
	cls->register_function(std::make_shared<ipc::function>("GetMultiplier", std::vector<ipc::type>{ipc::type::UInt64}, GetMultiplier));
	cls->register_function(std::make_shared<ipc::function>("SetMultiplier", std::vector<ipc::type>{ipc::type::UInt64, ipc::type::Float}, SetMultiplier));
	cls->register_function(std::make_shared<ipc::function>("Attach", std::vector<ipc::type>{ipc::type::UInt64, ipc::type::UInt64}, Attach));
	cls->register_function(std::make_shared<ipc::function>("Detach", std::vector<ipc::type>{ipc::type::UInt64}, Detach));
	srv.register_collection(cls);
}

void osn::Fader::ClearFaders()
{
	Manager::GetInstance().for_each([](obs_fader_t *fader) { obs_fader_destroy(fader); });

	Manager::GetInstance().clear();
}

void osn::Fader::Create(void *data, const int64_t id, const std::vector<ipc::value> &args, std::vector<ipc::value> &rval)
{
	obs_fader_type type = (obs_fader_type)args[0].value_union.i32;

	obs_fader_t *fader = obs_fader_create(type);
	if (!fader) {
		PRETTY_ERROR_RETURN(ErrorCode::Error, "Failed to create Fader.");
	}

	auto uid = Manager::GetInstance().allocate(fader);
	if (uid == std::numeric_limits<utility::unique_id::id_t>::max()) {
		obs_fader_destroy(fader);
		PRETTY_ERROR_RETURN(ErrorCode::CriticalError, "Failed to allocate unique id for Fader.");
	}

	rval.push_back(ipc::value((uint64_t)ErrorCode::Ok));
	rval.push_back(ipc::value(uid));
	AUTO_DEBUG;
}

void osn::Fader::Destroy(void *data, const int64_t id, const std::vector<ipc::value> &args, std::vector<ipc::value> &rval)
{
	auto uid = args[0].value_union.ui64;

	auto fader = Manager::GetInstance().find(uid);
	if (!fader) {
		PRETTY_ERROR_RETURN(ErrorCode::InvalidReference, "Invalid Fader Reference.");
	}

	obs_fader_destroy(fader);
	Manager::GetInstance().free(uid);

	rval.push_back(ipc::value((uint64_t)ErrorCode::Ok));
	AUTO_DEBUG;
}

void osn::Fader::GetDeziBel(void *data, const int64_t id, const std::vector<ipc::value> &args, std::vector<ipc::value> &rval)
{
	auto uid = args[0].value_union.ui64;

	auto fader = Manager::GetInstance().find(uid);
	if (!fader) {
		PRETTY_ERROR_RETURN(ErrorCode::InvalidReference, "Invalid Fader Reference.");
	}

	rval.push_back(ipc::value((uint64_t)ErrorCode::Ok));
	rval.push_back(ipc::value(obs_fader_get_db(fader)));
	AUTO_DEBUG;
}

void osn::Fader::SetDeziBel(void *data, const int64_t id, const std::vector<ipc::value> &args, std::vector<ipc::value> &rval)
{
	auto uid = args[0].value_union.ui64;

	auto fader = Manager::GetInstance().find(uid);
	if (!fader) {
		PRETTY_ERROR_RETURN(ErrorCode::InvalidReference, "Invalid Fader Reference.");
	}

	obs_fader_set_db(fader, args[1].value_union.fp32);

	rval.push_back(ipc::value((uint64_t)ErrorCode::Ok));
	rval.push_back(ipc::value(obs_fader_get_db(fader)));
	AUTO_DEBUG;
}

void osn::Fader::GetDeflection(void *data, const int64_t id, const std::vector<ipc::value> &args, std::vector<ipc::value> &rval)
{
	auto uid = args[0].value_union.ui64;

	auto fader = Manager::GetInstance().find(uid);
	if (!fader) {
		PRETTY_ERROR_RETURN(ErrorCode::InvalidReference, "Invalid Fader Reference.");
	}

	rval.push_back(ipc::value((uint64_t)ErrorCode::Ok));
	rval.push_back(ipc::value(obs_fader_get_deflection(fader)));
	AUTO_DEBUG;
}

void osn::Fader::SetDeflection(void *data, const int64_t id, const std::vector<ipc::value> &args, std::vector<ipc::value> &rval)
{
	auto uid = args[0].value_union.ui64;

	auto fader = Manager::GetInstance().find(uid);
	if (!fader) {
		PRETTY_ERROR_RETURN(ErrorCode::InvalidReference, "Invalid Fader Reference.");
	}

	obs_fader_set_deflection(fader, args[1].value_union.fp32);

	rval.push_back(ipc::value((uint64_t)ErrorCode::Ok));
	rval.push_back(ipc::value(obs_fader_get_deflection(fader)));
	AUTO_DEBUG;
}

void osn::Fader::GetMultiplier(void *data, const int64_t id, const std::vector<ipc::value> &args, std::vector<ipc::value> &rval)
{
	auto uid = args[0].value_union.ui64;

	auto fader = Manager::GetInstance().find(uid);
	if (!fader) {
		PRETTY_ERROR_RETURN(ErrorCode::InvalidReference, "Invalid Fader Reference.");
	}

	rval.push_back(ipc::value((uint64_t)ErrorCode::Ok));
	rval.push_back(ipc::value(obs_fader_get_mul(fader)));
	AUTO_DEBUG;
}

void osn::Fader::SetMultiplier(void *data, const int64_t id, const std::vector<ipc::value> &args, std::vector<ipc::value> &rval)
{
	auto uid = args[0].value_union.ui64;

	auto fader = Manager::GetInstance().find(uid);
	if (!fader) {
		PRETTY_ERROR_RETURN(ErrorCode::InvalidReference, "Invalid Fader Reference.");
	}

	obs_fader_set_mul(fader, args[1].value_union.fp32);

	rval.push_back(ipc::value((uint64_t)ErrorCode::Ok));
	rval.push_back(ipc::value(obs_fader_get_mul(fader)));
	AUTO_DEBUG;
}

void osn::Fader::Attach(void *data, const int64_t id, const std::vector<ipc::value> &args, std::vector<ipc::value> &rval)
{
	auto uid_fader = args[0].value_union.ui64;
	auto uid_source = args[1].value_union.ui64;

	auto fader = Manager::GetInstance().find(uid_fader);
	if (!fader) {
		PRETTY_ERROR_RETURN(ErrorCode::InvalidReference, "Invalid Fader Reference.");
	}

	auto source = osn::Source::Manager::GetInstance().find(uid_source);
	if (!source) {
		PRETTY_ERROR_RETURN(ErrorCode::InvalidReference, "Invalid Source Reference.");
	}

	if (!obs_fader_attach_source(fader, source)) {
		PRETTY_ERROR_RETURN(ErrorCode::Error, "Error attaching source..");
	}

	rval.push_back(ipc::value((uint64_t)ErrorCode::Ok));
	AUTO_DEBUG;
}

void osn::Fader::Detach(void *data, const int64_t id, const std::vector<ipc::value> &args, std::vector<ipc::value> &rval)
{
	auto uid = args[0].value_union.ui64;

	auto fader = Manager::GetInstance().find(uid);
	if (!fader) {
		PRETTY_ERROR_RETURN(ErrorCode::InvalidReference, "Invalid Fader Reference.");
	}

	obs_fader_detach_source(fader);

	rval.push_back(ipc::value((uint64_t)ErrorCode::Ok));
	AUTO_DEBUG;
}
